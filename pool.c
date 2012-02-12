#include "server.h"





void pool_init(struct pool_t *pool)
{
	int            i, fd;
	struct request *req;
	
	LL_INIT(&pool->free);
	pool->free_size = 0;
	
	//DEBUG_PRINT("%s:%d\r\n", pool->host, pool->port);
	
	for(i=0; i<srv.pool_min; i++)
	{
		fd            = anetNonConnect(&pool->addr);
		req           = &srv.req[fd];
		req->fd       = fd;
		req->vm       = NULL;
		req->buf      = NULL;
		req->link     = NULL;
		req->pool_id  = pool->id;
		req->state    = LINK_POOL;
		req->handler  = linkpool_close;
		
		if(req->fd == ANET_ERR)
		{
			//printf("can not connect to redis server %s:%d\r\n", pool->host, pool->port);
			
			exit(EXIT_FAILURE);
		}
		
		DEBUG_PRINT("req->fd = %d\r\n", req->fd);
		
		//�����¼�����,���Ӷ���ʱ��������ɾ������������
		srv.el->insert(req->fd, req, FDEVENT_READ);
		
		pool->free_size++;
		LL_TAIL(&pool->free, &req->entry);
	}
}


struct request *linkpool_get(const char *id)
{
	int    fd;
	struct pool_t  *pool;
	struct request *req;
	struct llhead  *lp, *tmp;
	
	
	pool = hash_get(backend, id);
	if(pool == NULL) return NULL;
	
	//DEBUG_PRINT("pool.free_size = %d\r\n", pool.free_size);
	
	if(pool->free_size == 0)
	{
		fd            = anetNonConnect(&pool->addr);
		req           = &srv.req[fd];
		req->fd       = fd;
		req->buf      = NULL;
		req->link     = NULL;
		req->state    = LINK_POOL;
		req->handler  = linkpool_close;
		req->pool_id  = pool->id;
		
		//DEBUG_PRINT("new req->fd = %d\r\n", req->fd);
		if(req->fd == ANET_ERR)
			return NULL;
		
		return req;
	}
	//������ӳ��п��е�����,��ֱ�ӷ�������
	LL_FOREACH_SAFE(&pool->free, lp, tmp) 
	{
		req = LL_ENTRY(lp, struct request, entry);
		if(req == NULL)
			continue;
			
		pool->free_size--;
			
		srv.el->delete(req->fd, req);
		//��free�������ͷ�,������used����,��������������ʹ��
		LL_DEL(&req->entry);
		
		req->pool_id = pool->id;
		return req;
	}
	
	return NULL;
}

void linkpool_free(struct request *req)
{
	struct pool_t  *pool;
	
	
	pool = hash_get(backend, req->pool_id);
	if(pool == NULL) return ;
	
	req->buf      = NULL;
	req->link     = NULL;
	req->state    = LINK_POOL;
	req->handler  = linkpool_close;
	
	srv.el->delete(req->fd, req);
	if(pool->free_size > srv.pool_max)
	{
		close(req->fd);
		req->fd    = 0;
		return ;
	}
	
	//�����¼�����,���Ӷ���ʱ��������ɾ������������
	srv.el->insert(req->fd, req, FDEVENT_READ);
	
	
	LL_TAIL(&pool->free, &req->entry);
	pool->free_size++;

	return ;
}

//�ر����ӳص�����
void linkpool_close(int fd, struct request *req)
{
	struct pool_t  *pool;
	
	
	pool = hash_get(backend, req->pool_id);
	if(pool == NULL) return ;
	
	//sdsfree(req->buf);
	close(req->fd);
	req->fd    = 0;
	req->buf   = NULL;
	req->link  = NULL;
	req->path  = NULL;
	
	srv.el->delete(req->fd, req);
	LL_DEL(&req->entry);
	pool->free_size--;

}
