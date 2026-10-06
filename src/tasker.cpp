#include "main.hpp"
#include <chrono>
namespace {
	using EM = spr::EffectManager;
	using namespace std::literals;
}
namespace spr {
	void EM::Tasker::workerLoop() {
		std::cout 
			<< "EffectManager.workerLoop: worker started"
			<< std::endl;
		_workerRunning = true;
		while (!_stopWorker) {
			Task task;
			{ std::unique_lock<std::mutex> lk(_queueMtx);
				if (waiting.empty()) {
					_queueCv.wait(lk, [this]() { return _stopWorker || !waiting.empty(); });
				}
				if (_stopWorker) break;
				if (waiting.empty()) continue;
				task = std::move(waiting.front());
				waiting.pop();
			}

			if (task.name.empty()) {// nothing to do
				if (task.data) { delete[] reinterpret_cast<char*>(task.data); task.data = nullptr; }
				continue;
			}

			std::cout
				<< "EffectManager.workerLoop: processing task '" + task.name + "' size=" + std::to_string(task.size)
				<< std::endl;

			// try to materialize effect; if an effect with same name exists, replace it
			// with a freshly created one; otherwise insert a new one.
			{
				auto existing = EM::instance().get(task.name);
				if (existing) {
					// create new effect first; only replace if creation succeeded
					Effect* newEff = nullptr;
					try {
						newEff = new Effect(task.name, task.data, task.size, task.filepath);
					} catch (...) {
						newEff = nullptr;
					}
					if (newEff && newEff->check()) {
						EM::instance().replace(task.name, newEff);
					} else {
						// creation failed; free and keep old one
						if (newEff) delete newEff;
					}
				} else {
					// not present: create/insert via existing API
					EM::instance().get_or_open(task.name, task.data, task.size, task.filepath);
				}
			}

			// free copied buffer (owned by task)
			if (task.data) { delete[] reinterpret_cast<char*>(task.data); task.data = nullptr; }
		}

		std::cout 
			<< "EffectManager.workerLoop: worker exiting"
			<< std::endl;
		_workerRunning = false;
	}

	void EM::Tasker::startWorkerIfNeeded(int delayms) {
		std::lock_guard<std::mutex> lk(_queueMtx);
		if (_stopWorker) return;
		if (_worker.joinable() && !_workerRunning) {
			try {
				_worker.join();
			} catch(const std::exception& e) {//...
			}
		}
		//ensure only one thread starts the worker
		if (!_worker.joinable()) {
			_stopWorker = false;
			try {
				_worker = std::thread([this, delayms]() { 
					if (delayms > 0) {
						std::this_thread::sleep_for(std::chrono::milliseconds(delayms));
					}
					this->workerLoop(); 
				});
			} catch (...) {
				_stopWorker = true;
				throw;
			}
		}
	}

	void EM::Tasker::stopWorker() {
		_stopWorker = true;
		_queueCv.notify_all();
		if (_worker.joinable()) {
			try {
				_worker.join();
			} catch (const std::exception& e) {//...
			}
			_worker = std::thread();
		}
		_workerRunning = false;
		// cleanup any pending tasks' buffers
		std::lock_guard<std::mutex> lk(_queueMtx);
		while (!waiting.empty()) {
			auto& t = waiting.front();
			if (t.data) { delete[] reinterpret_cast<char*>(t.data); t.data = nullptr; }
			waiting.pop();
		}
	}

	void EM::AsyncRequire(const Key& key, const void* ed, size_t es, const Path& fp) {
		// allocate copy if ed != nullptr and es > 0
		void* copyPtr = nullptr;
		if (ed && es > 0) {
			char* buf = new char[es];
			std::memcpy(buf, ed, es);
			copyPtr = buf;
		}
		{
			std::lock_guard<std::mutex> lk(tasker._queueMtx);
			tasker.waiting.emplace(key, fp, copyPtr, es);
		}
		if (*reinterpret_cast<HANDLE*>(0x89ffe0)) {//check setup finished: soku is ready!
			NotifyTasker();
		}
		std::cout
			<< "EffectManager.require_async: queued '" + key + "' size=" + std::to_string(es)
			<< std::endl;
	}
}
