
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#include <ngx_config.h>
#include <ngx_core.h>


ngx_int_t
ngx_thread_cond_create(ngx_thread_cond_t *cond, ngx_log_t *log)
{
    ngx_err_t  err;

    err = pthread_cond_init(cond, NULL);
    if (err == 0) {
        ngx_log_debug1(NGX_LOG_DEBUG_CORE, log, 0,
                       "pthread_cond_init(%p)", cond);
        return NGX_OK;
    }

    ngx_log_error(NGX_LOG_EMERG, log, err, "pthread_cond_init() failed");
    return NGX_ERROR;
}


ngx_int_t
ngx_thread_cond_destroy(ngx_thread_cond_t *cond, ngx_log_t *log)
{
    ngx_err_t  err;

    err = pthread_cond_destroy(cond);
    if (err == 0) {
        ngx_log_debug1(NGX_LOG_DEBUG_CORE, log, 0,
                       "pthread_cond_destroy(%p)", cond);
        return NGX_OK;
    }

    ngx_log_error(NGX_LOG_EMERG, log, err, "pthread_cond_destroy() failed");
    return NGX_ERROR;
}


ngx_int_t
ngx_thread_cond_signal(ngx_thread_cond_t *cond, ngx_log_t *log)
{
    ngx_err_t  err;

    err = pthread_cond_signal(cond);
    if (err == 0) {
        ngx_log_debug1(NGX_LOG_DEBUG_CORE, log, 0,
                       "pthread_cond_signal(%p)", cond);
        return NGX_OK;
    }

    ngx_log_error(NGX_LOG_EMERG, log, err, "pthread_cond_signal() failed");
    return NGX_ERROR;
}


ngx_int_t
ngx_thread_cond_wait(ngx_thread_cond_t *cond, ngx_thread_mutex_t *mtx,
    ngx_log_t *log)
{
    ngx_err_t  err;

    ngx_log_debug1(NGX_LOG_DEBUG_CORE, log, 0,
                   "pthread_cond_wait(%p) enter", cond);

/*
    pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t*mutex)��������Ĳ���mutex���ڱ�����������Ϊ�����ڵ���pthread_cond_waitʱ��
����������������Ǿͽ������������ǽ�����������ڼ䣬������������ı��˵Ļ��������Ǿ�©���������������Ϊ����̻߳�û�зŵ��ȴ������ϣ�
���Ե���pthread_cond_waitǰҪ������������������pthread_mutex_lock()��pthread_cond_wait�ڰ��̷߳Ž��������к��Զ���mutex���н�����
ʹ�������߳̿��Ի�ü�����Ȩ�������������̲߳��ܶ��ٽ���Դ���з��ʲ����ʵ���ʱ������������Ľ��̡���pthread_cond_wait���ص�ʱ��
���Զ���mutex������
ʵ���ϱߴ���ļӽ����������£�
/ ***********pthread_cond_wait()��ʹ�÷���********** /
pthread_mutex_lock(&qlock); / *lock* /
pthread_cond_wait(&qready, &qlock); / *block-->unlock-->wait() return-->lock* /
pthread_mutex_unlock(&qlock); / *unlock* /

*/
//pthread_cond_wait�ڲ�ִ��������:��������--����--�����̵߳ȴ����У��������(�����߳�ngx_thread_cond_signal)���ٴμ�����Ȼ�󷵻أ�
//Ϊʲô�ڵ��øú�����ʱ�����Ҫ�ȼ�����ԭ������pthread_cond_wait�ڲ���������״̬�и����̣���������������߳�cond signal�����߳�
//��ⲻ�����ź�signal�����������������ñ��߳̽���wait�̵߳ȴ����к󣬲����������߳�cond signal���ѱ��̣߳��Ϳ��Ա���©���ź�
    err = pthread_cond_wait(cond, mtx); //�ú����ڲ�ִ�й���:block-->unlock-->wait() return-->lock

#if 0
    ngx_time_update();
#endif

    if (err == 0) {
        ngx_log_debug1(NGX_LOG_DEBUG_CORE, log, 0,
                       "pthread_cond_wait(%p) exit", cond);
        return NGX_OK;
    }

    ngx_log_error(NGX_LOG_ALERT, log, err, "pthread_cond_wait() failed");

    return NGX_ERROR;
}
