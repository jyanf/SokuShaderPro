#include "main.hpp"
#include <chrono>
namespace {
	using EM = spr::EffectManager;
	using namespace std::literals;
}
namespace spr {
	void EM::Tasker::workerLoop() {
		std::this_thread::sleep_for(1000ms);//wait for minor requires
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

			// Try entering the D3D context lock; if fail, sleep and retry until success or stop requested
			//bool entered = false;
			//while (!_stopWorker) {
			//	if (TryEnterCriticalSection(&g_D3DContextLock)) {
			//		entered = true;
			//		break;
			//	}
			//	else {
			//		// try failed -> spec: wait 1s and retry
			//		std::cout << "EffectManager.workerLoop: TryEnterCriticalSection failed, sleeping 1s before retry for task '" + task.name + "'";
			//		std::this_thread::sleep_for(200ms);
			//	}
			//}
			//if (_stopWorker) {
			//	// cleanup buffer and break
			//	if (task.data) { delete[] reinterpret_cast<char*>(task.data); task.data = nullptr; }
			//	break;
			//}

			// try to materialize effect; if creation fails for reasons other than try-enter, do not auto-retry
			EM::instance().get_or_open(task.name, task.data, task.size, task.filepath);

			//LeaveCriticalSection(&g_D3DContextLock);

			// free copied buffer (owned by task)
			if (task.data) { delete[] reinterpret_cast<char*>(task.data); task.data = nullptr; }
		}

		std::cout 
			<< "EffectManager.workerLoop: worker exiting"
			<< std::endl;
		_workerRunning = false;
	}

	void EM::Tasker::startWorkerIfNeeded() {
		bool expect = false;
		if (!_workerRunning && !_stopWorker) {
			//ensure only one thread starts the worker
			std::lock_guard<std::mutex> lk(_queueMtx);
			if (!_worker.joinable()) {
				_stopWorker = false;
				_worker = std::thread([this]() { this->workerLoop(); });
				// detach is avoided; join in stopWorker / destructor
			}
		}
	}

	void EM::Tasker::stopWorker() {
		_stopWorker= true;
		_queueCv.notify_all();
		if (_worker.joinable()) {
			_worker.join();
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

	void EM::AsyncRequire(const Key& key, void* ed, size_t es, const Path& fp) {
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
