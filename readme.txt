 windmill: ����LUA�� redis��memcache��HandlerSocket��tokyotyrant���첽���
 
 author blog: http://www.cellphp.com/
 QQ group: 62116204
 email: lijinxing@gmail.com
 
 windmill �ص�
 1. ӵ�����ӳع���, ���ӿ��Ը���
 2. ����lua��coroutine�Ĳ����첽�Ƕ�����̡�
 
 
-- lua example code 
nicename   = redis.get("redis", "nicename");
name = redis.get("redis", "name");

redis.decr("redis", "flag");

age = redis.get("redis", "age");
str  = string.format("nicename=%s<br />name=%s<br />age=%s<br />",  nicename, name, age);


--redis.del("redis", "id", "name", "age");
sockio.echo(str);