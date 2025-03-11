#include "server.h"

void customTypeSetCommand(client *c)
{
    robj *o;
    // 1. 查找键，如果不存在则创建新的自定义对象
    if ((o = lookupKeyWrite(c->db, c->argv[1])) == NULL) {
        // 键不存在，创建新的自定义对象
        customTypeObject *nt = zmalloc(sizeof(customTypeObject));
        nt->id = 0;  // 初始值
        nt->name = sdsempty();  // 初始化为空 SDS 字符串

        // 封装为 Redis 对象
        robj *obj = createObject(OBJ_CUSTOM, nt);
        dbAdd(c->db, c->argv[1], obj);  // 将新对象添加到数据库
        o = obj;  // 更新指针
    } else {
        // 2. 如果键存在但类型不是自定义类型，返回错误
        if (o->type != OBJ_CUSTOM) {
            addReplyError(c, "Type mismatch");
            return;
        }
    }

    // 3. 处理参数并更新值
    customTypeObject *nt = o->ptr;
    if (getLongLongFromObjectOrReply(c, c->argv[2], &nt->id, "Invalid id") != C_OK) {
        return;
    }
    sds newname = c->argv[3]->ptr;
    if (nt->name) sdsfree(nt->name);  // 释放旧名称
    nt->name = sdsdup(newname);       // 复制新名称

    // 4. 返回成功响应
    addReply(c, shared.ok);
    signalModifiedKey(c, c->db, c->argv[1]);
    server.dirty++;
}

void customTypeGetCommand(client *c)
{
    robj *o = lookupKeyReadOrReply(c, c->argv[1], shared.null[c->resp]);
    if (o == NULL || o->type != OBJ_CUSTOM)
    {
        return;
    }
    customTypeObject *nt = o->ptr;
    if (nt->name == NULL) {
        addReplyError(c, "Invalid name field");
        return;
    }
    addReplyArrayLen(c, 2);
    addReplyLongLong(c, nt->id);
    addReplyBulkCString(c, nt->name);
}