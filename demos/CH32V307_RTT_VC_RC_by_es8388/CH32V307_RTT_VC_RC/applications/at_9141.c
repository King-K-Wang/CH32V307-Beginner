#include "ch32v30x.h"
#include <rtthread.h>
#include <rthw.h>
#include <at.h>

#define DRV_DEBUG
#define LOG_TAG              "drv.at"
#include <drv_log.h>

struct _ch9141_device
{
    rt_device_t device;
    rt_mutex_t  ch9141_lock;
};

static struct _ch9141_device  ch9141_device;




/* 可以用来操作板上9141，设置其模式，或者直接不设置，使用默认参数，另外一9141做主机  */
int at_send(int argc, char**argv)
{
    at_response_t resp = RT_NULL;
//    rt_mutex_take(ch9141_device.ch9141_lock, RT_WAITING_FOREVER);
    if (argc != 2)
    {
        rt_kprintf("at_cli_send [command]  - AT client send commands to AT server.");
        return -RT_ERROR;
    }
    /* 创建响应结构体，设置最大支持响应数据长度为 512 字节，响应数据行数无限制，超时时间为 5 秒 */
    resp = at_create_resp(512, 0, rt_tick_from_millisecond(5000));
    if (!resp)
    {
        rt_kprintf("No memory for response structure!");
        return -RT_ENOMEM;
    }

    /* 发送 AT 命令并接收响应数据，数据及信息存放在 resp 结构体中 */
    if (at_exec_cmd(resp, argv[1]) != RT_EOK)
    {
        rt_kprintf("AT client send commands failed, response error or timeout !");
        return -RT_ERROR;
    }
    /* 命令发送成功,打印返回字符串 */
    rt_kprintf("%s\r\n",resp->buf);
    at_delete_resp(resp);
//    rt_mutex_release(ch9141_device.ch9141_lock);
    return RT_EOK;
}
#ifdef FINSH_USING_MSH
#include <finsh.h>
MSH_CMD_EXPORT(at_send, AT Client send commands to AT Server and get response data);
#endif


rt_err_t rt_ch9141_get_device(const char *name)
{
    ch9141_device.device=rt_device_find(name);
    if(ch9141_device.device == RT_NULL)
    {
        return RT_ERROR;
    }
    else
    {
        rt_device_open(ch9141_device.device, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM);
    }
    return RT_EOK;
}

/* ch9141挂在uart7下 */
int ch9141_open(void)
{
    rt_err_t result=0;
    ch9141_device.ch9141_lock = rt_mutex_create("blelck", RT_IPC_FLAG_PRIO);
    if (ch9141_device.ch9141_lock == RT_NULL)
    {
        rt_kprintf("create dynamic mutex failed.\n");
        return RT_ERROR;
    }

    result=rt_ch9141_get_device("uart7");
    if(result == RT_ERROR)
    {
        rt_kprintf("open ch9141 error\r\n");
    }
    return result;
}
INIT_DEVICE_EXPORT(ch9141_open);


/* ch9141发送  */
void ch9141_send(rt_uint8_t *data, rt_uint16_t len)
{
//    rt_mutex_take(ch9141_device.ch9141_lock, RT_WAITING_FOREVER);
    rt_device_write(ch9141_device.device,0,data,len);
//    rt_mutex_release(ch9141_device.ch9141_lock);
}





