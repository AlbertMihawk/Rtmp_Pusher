//
// Created by lixiaoxu on 2019-08-25.
//

#ifndef HAWKEYEPLAYER_SAFE_QUEUE_H
#define HAWKEYEPLAYER_SAFE_QUEUE_H

#include <queue>
#include <pthread.h>

using namespace std;

template<typename T>
class SafeQueue {
    typedef void(*ReleaseCallback)(T *);
    typedef void(*SyncHandle)(queue<T> &);

public:
    SafeQueue() {
        pthread_mutex_init(&mutex, 0);
        pthread_cond_init(&cond, 0);
    }

    ~SafeQueue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    /**
     * 入队
     * @param value
     */
    void push(T value) {
        //加锁
        pthread_mutex_lock(&mutex);
        if (work) {
            //工作状态需要push
            q.push(value);
            pthread_cond_signal(&cond);
        } else {
            //非工作状态
            if (releaseCallback) {
                releaseCallback(&value);
            }
        }

        //解锁
        pthread_mutex_unlock(&mutex);
    }

    int pop(T &value) {
        int ret = 0;
        //加锁
        pthread_mutex_lock(&mutex);
        while (work && q.empty()) {
            //工作状态，需要pop，队列为空，需要等待
            pthread_cond_wait(&cond, &mutex);
        }
        if (!q.empty()) {
            value = q.front();
            q.pop();
            ret = 1;
        }
        //解锁
        pthread_mutex_unlock(&mutex);

        return ret;
    }

    /**
     * 设置队列工作状态
     * @param work
     */
    void setWork(int work) {
        //加锁
        pthread_mutex_lock(&mutex);
        this->work = work;
        pthread_cond_signal(&cond);
        //解锁
        pthread_mutex_unlock(&mutex);
    }

    /**
     * 判断是否为空
     * @return
     */
    int empty() {
        return q.empty();
    }

    /**
     * 队列大小
     * @return
     */
    int size() {
        return q.size();
    }

    /**
     * 清空队列
     * 队列中的元素如何释放？
     * AVPacket
     */
    void clear() {
        //加锁
        pthread_mutex_lock(&mutex);
        unsigned int size = q.size();
        for (int i = 0; i < size; ++i) {
            //取出对手元素
            T value = q.front();
            if (releaseCallback) {
                releaseCallback(&value);
            }
            q.pop();
        }

        //解锁
        pthread_mutex_unlock(&mutex);
    }

    void setReleaseCallback(ReleaseCallback releaseCallback) {
        this->releaseCallback = releaseCallback;
    }


    void setSyncHandle(SyncHandle syncHandle) {
        this->syncHandle = syncHandle;
    }
    //同步操作
    void sync(){
        //先锁起来
        pthread_mutex_lock(&mutex);
        syncHandle(q);
        //解锁
        pthread_mutex_unlock(&mutex);
    }


private:
    queue<T> q;
//互斥锁
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int work;//标记队列是否工作
    ReleaseCallback releaseCallback;
    SyncHandle syncHandle;
};

#endif //HAWKEYEPLAYER_SAFE_QUEUE_H
