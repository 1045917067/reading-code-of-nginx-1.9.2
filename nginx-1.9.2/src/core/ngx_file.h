
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_FILE_H_INCLUDED_
#define _NGX_FILE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

/*ʵ���ϣ�ngx_open_file��open���������𲻴�ngx_open_file���ص���Linuxϵͳ���ļ���������ڴ��ļ��ı�־λ��NginxҲ���������¼����������Է�װ��
#define NGX_FILE_RDONLY O_RDONLY
#define NGX_FILE_WRONLY O_WRONLY
#define NGX_FILE_RDWR O_RDWR
#define NGX_FILE_CREATE_OR_OPEN O_CREAT
#define NGX_FILE_OPEN 0
#define NGX_FILE_TRUNCATE O_CREAT|O_TRUNC
#define NGX_FILE_APPEND O_WRONLY|O_APPEND
#define NGX_FILE_NONBLOCK O_NONBLOCK

#define NGX_FILE_DEFAULT_ACCESS 0644
#define NGX_FILE_OWNER_ACCESS 0600

�ڴ��ļ�ʱֻ��Ҫ���ļ�·�����ݸ�name���������Ѵ򿪷�ʽ���ݸ�mode��create��access�������ɡ����磺
ngx_buf_t *b;
b = ngx_palloc(r->pool, sizeof(ngx_buf_t));

u_char* filename = (u_char*)"/tmp/test.txt";
b->in_file = 1;
b->file = ngx_pcalloc(r->pool, sizeof(ngx_file_t));
b->file->fd = ngx_open_file(filename, NGX_FILE_RDONLY|NGX_FILE_NONBLOCK, NGX_FILE_OPEN, 0);
b->file->log = r->connection->log;
b->file->name.data = filename;
b->file->name.len = sizeof(filename)-1;
if (b->file->fd <= 0)
{
 return NGX_HTTP_NOT_FOUND;
}

��������ʵ��û�н���������Ҫ��֪Nginx�ļ��Ĵ�С������������Ӧ�е�Content-Lengthͷ�����Լ�����ngx_buf_t��������file_pos��file_last��
ʵ���ϣ�ͨ��ngx_file_t�ṹ��ngx_file_info_t���͵�info�����Ϳ��Ի�ȡ�ļ���Ϣ��
typedef struct stat ngx_file_info_t;

Nginx��ֻ��stat���ݽṹ���˷�װ�������ɲ���ϵͳ�л�ȡ�ļ���Ϣ��stat������NginxҲʹ��һ��������˼򵥵ķ�װ��������ʾ��
#define ngx_file_info(file, sb)  stat((const char *) file, sb)

��ˣ���ȡ�ļ���Ϣʱ����������д��
if (ngx_file_info(filename, &b->file->info) == NGX_FILE_ERROR) {
 return NGX_HTTP_INTERNAL_SERVER_ERROR;
}

֮�����Ҫ����Content-Lengthͷ����
r->headers_out.content_length_n = b->file->info.st_size;

����Ҫ����ngx_buf_t��������file_pos��file_last��
b->file_pos = 0;
b->file_last = b->file->info.st_size;

�����Ǹ���Nginx���ļ���file_posƫ������ʼ�����ļ���һֱ����file_lastƫ��������ֹ��
ע�⡡���������д�����С�ļ�ʱ����ռ��Linux�ļ�ϵͳ�й����inode�ṹ����ʱ������Ľ������������С�ļ��ϲ���һ�����ļ�������������£�
������Ҫʱ��ֻҪ�������file_pos��file_last����Ϊ���ʵ�ƫ�������Ϳ���ֻ���ͺϲ����ļ��е�ĳһ�����ݣ�ԭ����С�ļ�����
�����Ϳ��Դ������С�ļ�������

Nginx���첽�ؽ������ļ���Ч�ط��͸��û����������Ǳ���Ҫ��HTTP�������Ӧ������Ϻ�ر��Ѿ��򿪵��ļ���������򽫻���־��й¶���⡣
���������ļ����Ҳ�ܼ򵥣�ֻ��Ҫ����һ��ngx_pool_cleanup_t�ṹ�壨������򵥵ķ�����HTTP��ܻ��ṩ��������ʽ�����������ʱ�ص�����HTTPģ���cleanup���������ڵ�11�½��ܣ���
�����Ǹյõ����ļ��������Ϣ������������Nginx�ṩ��ngx_pool_cleanup_file�������õ�����handler�ص������м��ɡ�

*/
struct ngx_file_s { //һ����Ϊngx_conf_file_t�ĳ�Աʹ�ã�
    /*
    fd�Ǵ��ļ��ľ�������������ļ���һ����Ҫ�û��Լ�������Nginx�򵥷�װ��һ������������openϵͳ�ĵ��ã�������ʾ��
    #define ngx_open_file(name, mode, create, access) open((const char *) name, mode|create, access)
    */
    ngx_fd_t                   fd;////�ļ����������
    ngx_str_t                  name;//�ļ�����
    ngx_file_info_t            info;//�ļ���С����Դ��Ϣ��ʵ�ʾ���Linuxϵͳ�����stat�ṹ

    /* ��ƫ��������Nginx���ڴ����ļ��δ��ˣ�һ�㲻����������Nginx��ܻ���ݵ�ǰ����״̬������ */
    off_t                      offset; //��ngx_read_file

    //��ǰ�ļ�ϵͳƫ������һ�㲻����������ͬ����Nginx�������
    off_t                      sys_offset; //��ngx_read_file

    ngx_log_t                 *log; //��־������ص���־�������logָ������־�ļ���

#if (NGX_THREADS)
    ngx_int_t                (*thread_handler)(ngx_thread_task_t *task,
                                               ngx_file_t *file);
    void                      *thread_ctx;
#endif

#if (NGX_HAVE_FILE_AIO)
    ngx_event_aio_t           *aio; //��ʼ��ֵ��ngx_file_aio_init
#endif

    unsigned                   valid_info:1;//Ŀǰδʹ��
    unsigned                   directio:1;//�������ļ��е�directio���������Ӧ���ڷ��ʹ��ļ�ʱ������Ϊ1
};


#define NGX_MAX_PATH_LEVEL  3


typedef time_t (*ngx_path_manager_pt) (void *data);
typedef void (*ngx_path_loader_pt) (void *data);

//�ο�ngx_conf_set_path_slot
typedef struct {
    ngx_str_t                  name; //·����
    size_t                     len;
    size_t                     level[3]; //·����3������

    ngx_path_manager_pt        manager; //�����Ƿ�����cache mange����  �ο�ngx_start_cache_manager_processes
    ngx_path_loader_pt         loader; //�����Ƿ�����cache loader����  �ο�ngx_start_cache_manager_processes
    void                      *data;

    u_char                    *conf_file;
    ngx_uint_t                 line;
} ngx_path_t;


typedef struct {
    ngx_str_t                  name;
    size_t                     level[3];
} ngx_path_init_t;


typedef struct { //ngx_http_write_request_body�лᴴ���ýṹ����ֵ
    ngx_file_t                 file; //��������ļ���Ϣ��fd �ļ�����
    off_t                      offset; //ָ��д�뵽�ļ��е����ݵ���β��
    ngx_path_t                *path; //�ļ�·��
    ngx_pool_t                *pool;
    char                      *warn; //��ʾ��Ϣ

    ngx_uint_t                 access; //�ļ�Ȩ�� 6660��

    unsigned                   log_level:8;  //��־�ȼ�request_body_file_log_level
    unsigned                   persistent:1; //�ļ������Ƿ����ô洢 request_body_in_persistent_file
    unsigned                   clean:1; //�ļ�ʱ��ʱ�ģ��ر����ӻ�ɾ���ļ���request_body_in_clean_file
} ngx_temp_file_t;


typedef struct {
    ngx_uint_t                 access;
    ngx_uint_t                 path_access;
    time_t                     time;
    ngx_fd_t                   fd;

    unsigned                   create_path:1;
    unsigned                   delete_file:1;

    ngx_log_t                 *log;
} ngx_ext_rename_file_t;


typedef struct {
    off_t                      size;
    size_t                     buf_size;

    ngx_uint_t                 access;
    time_t                     time;

    ngx_log_t                 *log;
} ngx_copy_file_t;


typedef struct ngx_tree_ctx_s  ngx_tree_ctx_t;

typedef ngx_int_t (*ngx_tree_init_handler_pt) (void *ctx, void *prev);
typedef ngx_int_t (*ngx_tree_handler_pt) (ngx_tree_ctx_t *ctx, ngx_str_t *name);

struct ngx_tree_ctx_s {
    off_t                      size;
    off_t                      fs_size;
    ngx_uint_t                 access;
    time_t                     mtime;

    ngx_tree_init_handler_pt   init_handler;
    ngx_tree_handler_pt        file_handler;
    ngx_tree_handler_pt        pre_tree_handler;
    ngx_tree_handler_pt        post_tree_handler;
    ngx_tree_handler_pt        spec_handler;

    void                      *data;
    size_t                     alloc;

    ngx_log_t                 *log;
};


ngx_int_t ngx_get_full_name(ngx_pool_t *pool, ngx_str_t *prefix,
    ngx_str_t *name);

ssize_t ngx_write_chain_to_temp_file(ngx_temp_file_t *tf, ngx_chain_t *chain);
ngx_int_t ngx_create_temp_file(ngx_file_t *file, ngx_path_t *path,
    ngx_pool_t *pool, ngx_uint_t persistent, ngx_uint_t clean,
    ngx_uint_t access);
void ngx_create_hashed_filename(ngx_path_t *path, u_char *file, size_t len);
ngx_int_t ngx_create_path(ngx_file_t *file, ngx_path_t *path);
ngx_err_t ngx_create_full_path(u_char *dir, ngx_uint_t access);
ngx_int_t ngx_add_path(ngx_conf_t *cf, ngx_path_t **slot);
ngx_int_t ngx_create_paths(ngx_cycle_t *cycle, ngx_uid_t user);
ngx_int_t ngx_ext_rename_file(ngx_str_t *src, ngx_str_t *to,
    ngx_ext_rename_file_t *ext);
ngx_int_t ngx_copy_file(u_char *from, u_char *to, ngx_copy_file_t *cf);
ngx_int_t ngx_walk_tree(ngx_tree_ctx_t *ctx, ngx_str_t *tree);

ngx_atomic_uint_t ngx_next_temp_number(ngx_uint_t collision);

char *ngx_conf_set_path_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_conf_merge_path_value(ngx_conf_t *cf, ngx_path_t **path,
    ngx_path_t *prev, ngx_path_init_t *init);
char *ngx_conf_set_access_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


extern ngx_atomic_t      *ngx_temp_number;
extern ngx_atomic_int_t   ngx_random_number;


#endif /* _NGX_FILE_H_INCLUDED_ */
