/*
 * ** Copyright (c) 2001-2003, S3 Graphics, Inc.
 * ** Copyright (c) 2004-2008, S3 Graphics Co., Ltd.
 * ** All Rights Reserved.
 * **
 * ** This is UNPUBLISHED PROPRIETARY SOURCE CODE of S3 Graphics, Inc.;
 * ** the contents of this file may not be disclosed to third parties, copied or
 * ** duplicated in any form, in whole or in part, without the prior written
 * ** permission of S3 Graphics, Inc.
 * **
 * ** RESTRICTED RIGHTS LEGEND:
 * ** Use, duplication or disclosure by the Government is subject to restrictions
 * ** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
 * ** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
 * ** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
 * ** rights reserved under the Copyright Laws of the United States.
 * */

#include "zx_disp.h"
#include "zx_cbios.h"
#include "zx_drmfb.h"
#include "zxgfx_trace.h"


/* cbios call back functions */
void disp_dbg_print(unsigned int debug_level, char *debug_message)
{
    zx_cb_printk(debug_message);
}

void disp_delay_micro_seconds(unsigned int usecs)
{
    if(usecs <= 20)
    {
        zx_udelay(usecs);
    }
    else if(usecs < 20000)
    {
        zx_usleep_range(usecs, usecs + (usecs >> 3));
    }
    else
    {
        zx_msleep(usecs/1000 + 1);
    }
}

unsigned char disp_read_uchar(void *data, unsigned int port)
{
    disp_info_t *disp_info = data;
    adapter_info_t* adapter = disp_info->adp_info;

    return zx_read8(adapter->mmio + port);
}

unsigned short disp_read_ushort(void *data, unsigned int port)
{
    disp_info_t *disp_info = data;
    adapter_info_t *adapter = disp_info->adp_info;

    return zx_read16(adapter->mmio + port);
}

unsigned int disp_read_ulong(void *data, unsigned int port)
{
    disp_info_t *disp_info = data;
    adapter_info_t *adapter = disp_info->adp_info;

    return zx_read32(adapter->mmio + port);
}

void disp_write_uchar(void *data, unsigned int port, unsigned char value)
{
    disp_info_t *disp_info = data;
    adapter_info_t *adapter = disp_info->adp_info;

    zx_write8(adapter->mmio + port, value);
}

void disp_write_ushort(void *data, unsigned int port, unsigned short value)
{
    disp_info_t *disp_info = data;
    adapter_info_t *adapter = disp_info->adp_info;

    zx_write16(adapter->mmio + port, value);
}

void disp_write_ulong(void *data, unsigned int port, unsigned int value)
{
    disp_info_t *disp_info = data;
    adapter_info_t *adapter = disp_info->adp_info;

    zx_write32(adapter->mmio + port, value);
}

void* disp_alloc_memory(unsigned int size)
{
    return zx_calloc(size);
}

void disp_free_pool(void *pool)
{
    zx_free(pool);
}

unsigned long long disp_acquire_spin_lock(void *spin_lock)
{
    struct os_spinlock *spin = (struct os_spinlock *)spin_lock;

    if (spin_lock == NULL)
    {
        zx_error("In func %s :first param spin_lock is NULL.\n", __func__);

        return 0;
    }

    return (unsigned long long)zx_spin_lock_irqsave(spin);
}

void disp_release_spin_lock(void *spin_lock, unsigned long long irq_status)
{
    struct os_spinlock *spin = (struct os_spinlock *)spin_lock;

    if (spin_lock == NULL)
    {
        zx_error("In func %s :first param spin_lock is NULL.\n", __func__);

        return;
    }

    zx_spin_unlock_irqrestore(spin, (unsigned long)irq_status);
}

void disp_acquire_mutex(void *mutex)
{
    struct os_mutex *m = (struct os_mutex *)mutex;

    if (m == NULL)
    {
        zx_error("In func %s :first param mutex is NULL.\n", __func__);

        return;
    }

    zx_mutex_lock(m);
}

void disp_release_mutex(void *mutex)
{
    struct os_mutex *m = (struct os_mutex *)mutex;

    if (m == NULL)
    {
        zx_error("In func %s :first param mutex is NULL.\n", __func__);

        return;
    }

    zx_mutex_unlock(m);
}

unsigned char disp_read_port_uchar(unsigned char *port)
{
    return zx_inb((unsigned short)(long)port);
}

void disp_write_port_uchar(unsigned char *port, unsigned char value)
{
    return zx_outb((unsigned short)(long)port, value);
}

void disp_write_log_file(unsigned int dbgleverl, unsigned char * dbgmsg, void * buffer, unsigned int size)
{
    //util_dump_memory_to_file(buffer, size, dbgmsg, CBIOS_LOG_FILE);
}

unsigned int disp_get_platform_config(void *data, const char* config_name, int *buffer, int length)
{
    disp_info_t *disp_info = data;
    zx_card_t*  zx_card = disp_info->zx_card;
    unsigned int ret = FALSE;

    if (!zx_get_platform_config(zx_card->pdev, config_name, buffer, length))
    {
        ret = TRUE;
    }
    else
    {
        ret = FALSE;
    }

    return ret;
}

void disp_query_sys_time(unsigned long long *u64_time)
{
    unsigned long sec = 0, usec = 0;

    if(u64_time == NULL)
    {
        return ;
    }

    zx_getsecs(&sec, &usec);

    *u64_time = (unsigned long long)sec * 1000000 + usec;
}

int  disp_get_output_num(int  outputs)
{
    int  num = 0;
    while(outputs)
    {
        if(outputs & 1)
        {
            num++;
        }
        outputs >>= 1;
    }
    return  num;
}

int  disp_biosmonitor_to_output(int bios_monitor)
{
    int  output = 0;
    if(bios_monitor & CBIOS_MONITOR_TYPE_CRT)
    {
        output |= UT_OUTPUT_TYPE_CRT;
    }
    if(bios_monitor & CBIOS_MONITOR_TYPE_TV)
    {
        output |= UT_OUTPUT_TYPE_TV;
    }
    if(bios_monitor & CBIOS_MONITOR_TYPE_HDTV)
    {
        output |= UT_OUTPUT_TYPE_HDTV;
    }
    if(bios_monitor & CBIOS_MONITOR_TYPE_PANEL)
    {
        output |= UT_OUTPUT_TYPE_PANEL;
    }
    if(bios_monitor & CBIOS_MONITOR_TYPE_DVI)
    {
        output |= UT_OUTPUT_TYPE_DVI;
    }
    if(bios_monitor & CBIOS_MONITOR_TYPE_HDMI)
    {
        output |= UT_OUTPUT_TYPE_HDMI;
    }
    if(bios_monitor & CBIOS_MONITOR_TYPE_DP)
    {
        output |= UT_OUTPUT_TYPE_DP;
    }
    if(bios_monitor & CBIOS_MONITOR_TYPE_MHL)
    {
        output |= UT_OUTPUT_TYPE_MHL;
    }

    return output;
}

int  disp_output_to_biosmonitor(int output)
{
    int  bios_monitor = 0;
    if(output & UT_OUTPUT_TYPE_CRT)
    {
        bios_monitor |= CBIOS_MONITOR_TYPE_CRT;
    }
    if(output & UT_OUTPUT_TYPE_TV)
    {
        bios_monitor |= CBIOS_MONITOR_TYPE_TV;
    }
    if(output & UT_OUTPUT_TYPE_HDTV)
    {
        bios_monitor |= CBIOS_MONITOR_TYPE_HDTV;
    }
    if(output & UT_OUTPUT_TYPE_PANEL)
    {
        bios_monitor |= CBIOS_MONITOR_TYPE_PANEL;
    }
    if(output & UT_OUTPUT_TYPE_DVI)
    {
        bios_monitor |= CBIOS_MONITOR_TYPE_DVI;
    }
    if(output & UT_OUTPUT_TYPE_HDMI)
    {
        bios_monitor |= CBIOS_MONITOR_TYPE_HDMI;
    }
    if(output & UT_OUTPUT_TYPE_DP)
    {
        bios_monitor |= CBIOS_MONITOR_TYPE_DP;
    }
    if(output & UT_OUTPUT_TYPE_MHL)
    {
        bios_monitor |= CBIOS_MONITOR_TYPE_MHL;
    }

    return bios_monitor;    
}

CHIPID_HW disp_chipid_to_cbios_chipid(unsigned int chip_id)
{
     CHIPID_HW cb_chip_id = 0;

     switch(chip_id)
     {
     case CHIP_DST2:
         cb_chip_id = CHIPID_DST2;
         break;
     case CHIP_DST3:
         cb_chip_id = CHIPID_DST3;
         break;
     case CHIP_DST4:
         cb_chip_id = CHIPID_DST4;
         break;
     case CHIP_DUMA:
         cb_chip_id = CHIPID_DUMA;
         break;
     case CHIP_E2UMA:
         cb_chip_id = CHIPID_E2UMA;
         break;
     case CHIP_ELT:
     case CHIP_ELT1K:
         cb_chip_id = CHIPID_ELITE;
         break;
     case CHIP_ELT2K:
         cb_chip_id = CHIPID_ELITE2K;
         break;
     case CHIP_ZX2000:
         cb_chip_id = CHIPID_ZX2K;
         break;
     case CHIP_CHX001:
         cb_chip_id = CHIPID_CHX001;
         break;
     case CHIP_CHX002:
         cb_chip_id = CHIPID_CHX002;
         break;
     default:
         zx_assert(0);
         break;
     }

     return cb_chip_id;
}

/*
 * @en_linear   : if 1, enable linear.
 * @en_vga      : if 0, prevent vga console accessing fb
 *
 * return       : current linear status
 */
int disp_enable_disable_linear_vga(disp_info_t* disp_info, int en_linear, int en_vga)
{
    adapter_info_t*  adapter_info = disp_info->adp_info;
    unsigned char  cr_a0_c = zx_read8(adapter_info->mmio + 0x8AA0);
    int status = (cr_a0_c & 0x10) ? 1 : 0;

    //if no conflict between linear and vga, we always keep linear and vga enable
    if(!disp_info->conflict_in_linear_vga)
    {
        en_linear = en_vga = 1;
    }

    if(en_linear)
    {
        cr_a0_c |= 0x10;
    }
    else
    {
        cr_a0_c &= ~0x10;
    }
    zx_write8(adapter_info->mmio + 0x8AA0, cr_a0_c);

    if(en_vga && !disp_info->vga_enabled)
    {
        console_unlock();
        disp_info->vga_enabled = 1;
    }
    else if(!en_vga && disp_info->vga_enabled)
    {
        console_lock();
        disp_info->vga_enabled= 0;
    }

    return status;
}

int disp_get_shadow_rom_image(disp_info_t *disp_info)
{
    void *shadow_rom_image = NULL;
    void *src_image = NULL;
    unsigned char sr_1f;
    unsigned int mm850c = 0, vid_boundary = 0, bound_reg = 0;
    adapter_info_t*  adapter_info = disp_info->adp_info;
    zx_map_argu_t    map  = {0};
    zx_vm_area_t     *vma = NULL;
    int linear_status;
    
    if((adapter_info->run_on_qt))
    {
        return 0;
    }

    vid_boundary = ((adapter_info->fb_total_size >> 24) -1 ) & 0xFF;
    bound_reg = zx_read32(adapter_info->mmio + 0x490a0);
    zx_write32(adapter_info->mmio + 0x490a0, (bound_reg & 0xFFFFFF00) | vid_boundary);

    shadow_rom_image = zx_calloc(ZX_SHADOW_VBIOS_SIZE);
    if(!shadow_rom_image)
    {
        zx_error("malloc shadow rom memory failed\n");
        goto fail;
    }

    map.flags.cache_type = ZX_MEM_UNCACHED;
    map.flags.mem_space  = ZX_MEM_KERNEL;
    map.flags.mem_type   = ZX_SYSTEM_IO;
    map.phys_addr        = adapter_info->fb_bus_addr + adapter_info->fb_total_size - ZX_SHADOW_VBIOS_SIZE;
    map.size             = ZX_SHADOW_VBIOS_SIZE;

    vma = zx_map_io_memory(NULL, &map);

    src_image = vma->virt_addr;
    if(!src_image)
    {
        zx_error("map shadow vbios failed\n");
        goto fail;
    }
    
    /* enable eclk */
    sr_1f = zx_read8(adapter_info->mmio + 0x861F);
    zx_write8(adapter_info->mmio + 0x861F, sr_1f & 0xfe);

    mm850c = zx_read32(adapter_info->mmio + 0x850c);
    if(!(mm850c & 0x2))
    {
        zx_write8(adapter_info->mmio + 0x850c, mm850c | 0x2);
    }

    linear_status = disp_enable_disable_linear_vga(disp_info, 1, 0);

    zx_memcpy(shadow_rom_image, src_image, ZX_SHADOW_VBIOS_SIZE);

    zx_unmap_io_memory(vma);

    /* restore cr_a0_c */
    zx_write32(adapter_info->mmio + 0x850c, mm850c);

    disp_enable_disable_linear_vga(disp_info, linear_status, 1);

    if(*(unsigned short*)shadow_rom_image != 0xAA55)
    {
        zx_error("Invalid shadow rom_image \n");
        goto fail;
    }

    disp_info->rom_image = shadow_rom_image;

    return  ZX_SHADOW_VBIOS_SIZE;

fail:
    if(shadow_rom_image)
    {
        zx_free(shadow_rom_image);
        shadow_rom_image = NULL;
    }

    return  0;
}

int disp_wait_for_vblank(disp_info_t* disp_info, int pipe, int timeout)
{
    unsigned long timeout_j = jiffies + msecs_to_jiffies(timeout) + 1;
    unsigned int ori_vblcnt = 0, curr_vblcnt = 0;
    int ret = 0;
    zx_get_counter_t   get_cnt = {0};

    get_cnt.crtc_index = pipe;
    get_cnt.vblk = &ori_vblcnt;

    disp_cbios_get_counter(disp_info, &get_cnt);

    curr_vblcnt = ori_vblcnt;
    get_cnt.vblk = &curr_vblcnt;

    while(curr_vblcnt == ori_vblcnt)
    {
        if(time_after(jiffies, timeout_j))
        {
            ret = -ETIMEDOUT;
            break;
        }
        if(drm_can_sleep())
        {
            zx_msleep(1);
        }
        else
        {
            zx_udelay(1000);
        }
        disp_cbios_get_counter(disp_info, &get_cnt);
    }

    return  ret;
}

/*CBIOS Initialization sequence:
 * 1) set Call back function
 * 2) Set Mmio Endian Mode
 * 3) CbiosInit
 * 4) CbiosInitHW
 * Before 2, we can't let CBIOS access MMIO as we don't set MMIO mode as we required
 * */
int disp_init_cbios(disp_info_t *disp_info)
{
    CBIOS_PARAM_INIT                 CBParamInit = {0};
    CBIOS_CALLBACK_FUNCTIONS fnCallBack = {0};
    CBIOS_CHIP_ID CBChipId;
    adapter_info_t*  adapter_info = disp_info->adp_info;
    void *pcbe = NULL;
    int  rom_lenth = 0;
    unsigned int CBiosStatus;
    unsigned int CBiosExtensionSize;
    unsigned int i = 0;

    fnCallBack.Size = sizeof(CBIOS_CALLBACK_FUNCTIONS);

    fnCallBack.pFnDbgPrint          = disp_dbg_print;
    fnCallBack.pFnDelayMicroSeconds = disp_delay_micro_seconds;
    fnCallBack.pFnReadUchar         = disp_read_uchar;
    fnCallBack.pFnReadUshort        = disp_read_ushort;
    fnCallBack.pFnReadUlong         = disp_read_ulong;
    fnCallBack.pFnWriteUchar        = disp_write_uchar;
    fnCallBack.pFnWriteUshort       = disp_write_ushort;
    fnCallBack.pFnWriteUlong        = disp_write_ulong;
    fnCallBack.pFnQuerySystemTime   = disp_query_sys_time;
    fnCallBack.pFnAllocateNonpagedMemory = disp_alloc_memory;
    fnCallBack.pFnAllocatePagedMemory= disp_alloc_memory;
    fnCallBack.pFnFreePool          = disp_free_pool;
    fnCallBack.pFnAcquireSpinLock   = disp_acquire_spin_lock;
    fnCallBack.pFnReleaseSpinLock   = disp_release_spin_lock;
    fnCallBack.pFnAcquireMutex      = disp_acquire_mutex;
    fnCallBack.pFnReleaseMutex      = disp_release_mutex;
    fnCallBack.pFnReadPortUchar     = disp_read_port_uchar;
    fnCallBack.pFnWritePortUchar    = disp_write_port_uchar;
    fnCallBack.pFnDbgPrintToFile    = disp_write_log_file;
    fnCallBack.pFnGetPlatformConfigU32 = disp_get_platform_config;

    fnCallBack.pFnStrcmp            = zx_strcmp;
    fnCallBack.pFnStrcpy            = zx_strcpy;
    fnCallBack.pFnStrncmp           = zx_strncmp;
    fnCallBack.pFnMemset            = zx_memset;
    fnCallBack.pFnMemcpy            = zx_memcpy;
    fnCallBack.pFnMemcmp            = zx_memcmp;
    fnCallBack.pFnDodiv             = zx_do_div;
    fnCallBack.pFnVsprintf          = zx_vsprintf;
    fnCallBack.pFnVsnprintf         = zx_vsnprintf;

    if(CBiosSetCallBackFunctions(&fnCallBack) != CBIOS_OK)
    {
        zx_info("CBios set call back func failed.\n");
    }

    if((adapter_info->chip_id == CHIP_E2UMA) || 
        (adapter_info->chip_id == CHIP_CHX001)  ||
        (adapter_info->chip_id == CHIP_CHX002))
    {
        rom_lenth = disp_get_shadow_rom_image(disp_info);
    }

    zx_memset(&CBChipId, 0, sizeof(CBIOS_CHIP_ID));
    CBChipId.Size = sizeof(CBIOS_CHIP_ID);
    CBChipId.GenericChipID = adapter_info->generic_id;
    CBChipId.ChipID = disp_chipid_to_cbios_chipid(adapter_info->chip_id);

    CBiosStatus = CBiosGetExtensionSize(&CBChipId, &CBiosExtensionSize);

    if(CBiosExtensionSize == 0 || CBiosStatus != CBIOS_OK)
    {
        zx_error("Get cbios extension size failed\n");
    }

    pcbe = zx_calloc(CBiosExtensionSize);

    if(pcbe == NULL)
    {
        zx_error("Alloc memory for cbios pcbe failed\n");
    }

    CBParamInit.pAdapterContext    = disp_info;
    CBParamInit.MAMMPrimaryAdapter = adapter_info->primary;
    CBParamInit.GeneralChipID      = adapter_info->generic_id;
    CBParamInit.ChipID             = disp_chipid_to_cbios_chipid(adapter_info->chip_id);
    CBParamInit.SVID               = adapter_info->sub_sys_vendor_id;
    CBParamInit.SSID               = adapter_info->sub_sys_id;
    CBParamInit.RomImage           = disp_info->rom_image;
    CBParamInit.RomImageLength     = rom_lenth;
    CBParamInit.pSpinLock          = disp_info->cbios_inner_spin_lock;
    CBParamInit.pAuxMutex       = disp_info->cbios_aux_mutex;

    for(i = 0;i < CBIOS_MAX_I2CBUS;i++)
    {
        CBParamInit.pI2CMutex[i] = disp_info->cbios_i2c_mutex[i];
    }
    
    CBParamInit.Size               = sizeof(CBIOS_PARAM_INIT);

#ifdef GFX_ONLY_FPGA
    CBParamInit.bRunOnQT           = 0x1;
#endif

    if (adapter_info->run_on_qt)
    {
        CBParamInit.bRunOnQT           = 0x1;
    }

    CBiosInit(pcbe, &CBParamInit);

    disp_info->cbios_ext = pcbe;

    return S_OK;
}

int disp_cbios_init_hw(disp_info_t *disp_info)
{
    int                    ret        = S_OK;
    int                    cb_status  = CBIOS_OK;

    cb_status = CBiosInitHW(disp_info->cbios_ext, NULL);

    if(cb_status != CBIOS_OK)
    {
        zx_error("CBiosInitHW failed. cbios status is %x\n",cb_status);
        ret = E_FAIL;
    }

    zx_info("disp cbios_init_hw finished.\n");
    
    return ret;
}

void disp_cbios_get_crtc_resource(disp_info_t *disp_info)
{
    CBIOS_GET_DISP_RESOURCE   cb_disp_res = {0};
    int  i = 0;

    if(CBIOS_OK == CBiosGetDispResource(disp_info->cbios_ext, &cb_disp_res))
    {
        zx_assert(MAX_CORE_CRTCS >=  cb_disp_res.CrtcNum);
        
        disp_info->num_crtc = cb_disp_res.CrtcNum;

        for(i = 0; i < cb_disp_res.CrtcNum; i++)
        {
            disp_info->num_plane[i]= cb_disp_res.PlaneNum[i];
        }
    }
}

void disp_cbios_get_crtc_caps(disp_info_t *disp_info)
{  
    CBIOS_DISPLAY_CAPS cb_disp_caps = {0};
    CBIOS_U32      up_plane[MAX_CORE_CRTCS] = {0};
    CBIOS_U32      down_plane[MAX_CORE_CRTCS] = {0};
    int i = 0;
    
    cb_disp_caps.pUpScalePlaneMask = up_plane;
    cb_disp_caps.pDownScalePlaneMask = down_plane;
    
    if(CBIOS_OK == CBiosGetDisplayCaps(disp_info->cbios_ext, &cb_disp_caps))
    {
        disp_info->scale_support = cb_disp_caps.UpScaleSupport;   //only use crtc upscaler
        for(i = 0; i < disp_info->num_crtc; i++)
        {
            disp_info->up_scale_plane_mask[i] = up_plane[i];
        }
        for(i = 0; i < disp_info->num_crtc; i++)
        {
            disp_info->down_scale_plane_mask[i] = down_plane[i];
        }
    }
}

int disp_cbios_cleanup(disp_info_t *disp_info)
{
    CBiosUnload(disp_info->cbios_ext);

    if(disp_info->cbios_ext)
    {
        zx_free(disp_info->cbios_ext);

        disp_info->cbios_ext = NULL;
    }

    if(disp_info->rom_image)
    {
        zx_free(disp_info->rom_image);

        disp_info->rom_image = NULL;
    }

    return S_OK;
}

int disp_sync_data_with_Vbios(disp_info_t * disp_info, unsigned int SyncToVBios)
{
    void* pcbe = disp_info->cbios_ext; 
    CBIOS_VBIOS_DATA_PARAM CBiosSyncWithVBiosParam = {0};
    int                    ret        = S_OK;
    int                    cb_status  = CBIOS_OK;
    int                    status = CBIOS_OK;

    CBiosSyncWithVBiosParam.Size = sizeof(CBIOS_VBIOS_DATA_PARAM);
    CBiosSyncWithVBiosParam.SyncToVbios = SyncToVBios;

    cb_status = CBiosSyncDataWithVbios(pcbe,&CBiosSyncWithVBiosParam);
    if (cb_status != CBIOS_OK)
    {
        zx_error( " disp sync data with vbios failed\n");
        ret = E_FAIL;
    }
    else
    {
        CIP_ACTIVE_DEVICES active_device = {0};
        //get vbios active device 
        zx_memset(&active_device, 0, sizeof(CIP_ACTIVE_DEVICES));
        status = CBiosGetActiveDevice(pcbe, &active_device);
        
        if(status != CBIOS_OK)
        {
            zx_error("Get Active Device failed\n");
            ret = E_FAIL;
        }
        else
        {
            int i = 0;
            for(i = 0; i < MAX_CORE_CRTCS; i++)
            {
                disp_info->active_output[i] = active_device.DeviceId[i];
            }
            zx_info("Vbios active output iga1 0x%x iga2 0x%x iga3 0x%x\n", 
                                 disp_info->active_output[IGA1],
                                 disp_info->active_output[IGA2],
                                 disp_info->active_output[IGA3]);
        }
    }

    return ret;
     
}


void disp_cbios_query_vbeinfo(disp_info_t *disp_info)
{
    adapter_info_t* adapter_info = disp_info->adp_info;
    void* pcbe = disp_info->cbios_ext;
    CBIOS_VBINFO_PARAM vbeinfo = {0};
    int status = CBIOS_OK;

    vbeinfo.Size = sizeof(vbeinfo);
    vbeinfo.BiosVersion = CBIOSVERSION;

    status = CBiosGetVBiosInfo(pcbe, &vbeinfo);
    if(status != CBIOS_OK)
    {
        zx_error("Get cbios vbeinfo failed\n");
    }
    else
    {
        if(adapter_info->chip_id == CHIP_CHX001 || adapter_info->chip_id == CHIP_CHX002)
        {
            adapter_info->low_top_addr = vbeinfo.LowTopAddress;
            adapter_info->snoop_only = vbeinfo.SnoopOnly;
            adapter_info->ta_enable  = vbeinfo.bTAEnable;

            if(adapter_info->run_on_hypervisor)
            {
                adapter_info->snoop_only = 1;
            }

            adapter_info->avai_mem_size_mb = vbeinfo.FBSize;
        }
       
        disp_info->support_output = vbeinfo.SupportDev;
        disp_info->num_output = disp_get_output_num(vbeinfo.SupportDev);

        disp_info->supp_hpd_outputs   = disp_info->support_output & vbeinfo.HPDDevicesMask;
        disp_info->supp_polling_outputs = disp_info->support_output & vbeinfo.PollingDevMask;

        disp_info->vbios_version = vbeinfo.BiosVersion;

        disp_info->dp5_conn_type = vbeinfo.DP5ConnectorType;
        disp_info->dp6_conn_type = vbeinfo.DP6ConnectorType;

        disp_info->bl_gfx_mode = vbeinfo.bBlGfxMode;
      
        zx_info("bios supported device: 0x%x\n", disp_info->support_output);
        zx_info("low_top_address : 0x%x\n", adapter_info->low_top_addr);
        zx_info("snoop only: %d\n", adapter_info->snoop_only);
        zx_info("ta_enable (iov enable): %d\n", adapter_info->ta_enable);
        zx_info("Backlight Gfx Mode: %d\n", disp_info->bl_gfx_mode);
    }
    
}

int disp_cbios_get_modes_size(disp_info_t *disp_info, int output)
{
    void              *pcbe = disp_info->cbios_ext;
    int                 mode_size = 0;

    if(CBIOS_OK == CBiosGetDeviceModeListBufferSize(pcbe, output, &mode_size))
    {
        if(mode_size > 0)
        {
            mode_size += 10 * sizeof(CBiosModeInfoExt); //add some space to avoid overflow in get mode list later
        }
        else
        {
            mode_size = 0;
        }
    }

    return  mode_size;
}

int disp_cbios_get_modes(disp_info_t *disp_info, int output, void* buffer, int buf_size)
{
    void                *pcbe = disp_info->cbios_ext;
    int                 real_size = buf_size;
    int                 real_num = 0;

    if(!buffer || !buf_size)
    {
        return 0;
    }
    
    CBiosGetDeviceModeList(pcbe, output, (PCBiosModeInfoExt)buffer, &real_size);

    if(real_size > buf_size)
    {
        zx_error("OVERFLOW detected: malloc_size: %d, used size: %x.\n", buf_size, real_size);
    }

    real_num = real_size / sizeof(CBiosModeInfoExt);

    return  real_num;
}

int disp_cbios_get_adapter_modes_size(disp_info_t *disp_info)
{
    void              *pcbe = disp_info->cbios_ext;
    int                 mode_size = 0;

    if(CBIOS_OK == CBiosGetAdapterModeListBufferSize(pcbe, &mode_size))
    {
        if(mode_size > 0)
        {
            mode_size += 10 * sizeof(CBiosModeInfoExt); //add some space to avoid overflow in get mode list later
        }
        else
        {
            mode_size = 0;
        }
    }

    return  mode_size;
}

int disp_cbios_get_adapter_modes(disp_info_t *disp_info, void* buffer, int buf_size)
{
    void                *pcbe = disp_info->cbios_ext;
    int                 real_size = buf_size;
    int                 real_num = 0;

    if(!buffer || !buf_size)
    {
        return 0;
    }
    
    CBiosGetAdapterModeList(pcbe, (PCBiosModeInfoExt)buffer, &real_size);

    if(real_size > buf_size)
    {
        zx_error("OVERFLOW detected: malloc_size: %d, used size: %x.\n", buf_size, real_size);
    }

    real_num = real_size / sizeof(CBiosModeInfoExt);

    return  real_num;
}

int disp_cbios_merge_modes(CBiosModeInfoExt* merge_mode_list, CBiosModeInfoExt * adapter_mode_list, unsigned int const adapter_mode_num, 
    CBiosModeInfoExt const * dev_mode_list, unsigned int const dev_mode_num)
{
    CBiosModeInfoExt    tmpBuf;
    unsigned int        i = 0;
    unsigned int        adapter_index      = 0;
    unsigned int        dev_index          = 0;
    unsigned int        mode_index         = 0;
    unsigned int        num_mode           = 0;
    unsigned int        *valid_adapter_mode_flag = NULL;


    valid_adapter_mode_flag = zx_calloc(adapter_mode_num * sizeof(unsigned int ));

    // reverse adapter modelist
    for (i = 0; i < adapter_mode_num/2; i++)
    {
        zx_memcpy(&tmpBuf, &adapter_mode_list[i], sizeof(CBiosModeInfoExt));
        zx_memcpy(&adapter_mode_list[i], &adapter_mode_list[adapter_mode_num - i -1], sizeof(CBiosModeInfoExt));
        zx_memcpy(&adapter_mode_list[adapter_mode_num - i -1], &tmpBuf, sizeof(CBiosModeInfoExt));
    }

    //make sure all the adapter modes is smaller than the largest device mode.
    //adapter mode is not strict sorted from small to large .
    //adatper modes : 640x480 800x600 1024x768 1280x720 1280x1024 1680x1050 1920x1080
    //1024x768 is not smaller than 1280x720 strictly.

    // mark adapter mode whether it's bigger than the largest device mode or not
    for (i = 0; i < adapter_mode_num; i++)
    {
        if ((adapter_mode_list[i].XRes <= dev_mode_list[0].XRes) && 
         (adapter_mode_list[i].YRes <= dev_mode_list[0].YRes) &&
         (adapter_mode_list[i].RefreshRate <= dev_mode_list[0].RefreshRate))
        {
            valid_adapter_mode_flag[i] = 1;
        }
        else
        {
            valid_adapter_mode_flag[i] = 0;
        }
    }

    // merge
    dev_index  = 0;
    mode_index = 0;
    num_mode   = 0;
    adapter_index = 0;
    while ((dev_index < dev_mode_num) && (adapter_index < adapter_mode_num))
    {
        if(valid_adapter_mode_flag[adapter_index] == 0)
        {
            adapter_index++;
            continue;
        }
        if ((dev_mode_list[dev_index].XRes > adapter_mode_list[adapter_index].XRes) ||
            ((dev_mode_list[dev_index].XRes == adapter_mode_list[adapter_index].XRes) &&
             (dev_mode_list[dev_index].YRes > adapter_mode_list[adapter_index].YRes)) ||
            ((dev_mode_list[dev_index].XRes == adapter_mode_list[adapter_index].XRes) &&
             (dev_mode_list[dev_index].YRes == adapter_mode_list[adapter_index].YRes) &&
             (dev_mode_list[dev_index].RefreshRate > adapter_mode_list[adapter_index].RefreshRate)))
        {
            zx_memcpy(&merge_mode_list[mode_index], &dev_mode_list[dev_index], sizeof(CBiosModeInfoExt));
            mode_index++;
            dev_index++;
            num_mode++;
        }
        else if ((dev_mode_list[dev_index].XRes == adapter_mode_list[adapter_index].XRes) &&
                 (dev_mode_list[dev_index].YRes == adapter_mode_list[adapter_index].YRes) &&
                 (dev_mode_list[dev_index].RefreshRate == adapter_mode_list[adapter_index].RefreshRate))
        {
            zx_memcpy(&merge_mode_list[mode_index], &dev_mode_list[dev_index], sizeof(CBiosModeInfoExt));
            merge_mode_list[mode_index].InterlaceProgressiveCaps |= adapter_mode_list[adapter_index].InterlaceProgressiveCaps;
            merge_mode_list[mode_index].DeviceFlags |= adapter_mode_list[adapter_index].DeviceFlags;
            merge_mode_list[mode_index].ColorDepthCaps |= adapter_mode_list[adapter_index].ColorDepthCaps;
            merge_mode_list[mode_index].AspectRatioCaps |= adapter_mode_list[adapter_index].AspectRatioCaps;
            mode_index++;
            dev_index++;
            adapter_index++;
            num_mode++;
        }
        else
        {
            zx_memcpy(&merge_mode_list[mode_index], &adapter_mode_list[adapter_index], sizeof(CBiosModeInfoExt));
            mode_index++;
            adapter_index++;
            num_mode++;
        }
    }

    if ((dev_index == dev_mode_num) && (adapter_index < adapter_mode_num))
    {
        while(adapter_index < adapter_mode_num)
        {
            if(valid_adapter_mode_flag[adapter_index] == 1)
            {
                zx_memcpy(&merge_mode_list[mode_index], &adapter_mode_list[adapter_index], sizeof(CBiosModeInfoExt));
                num_mode++;
                mode_index++;
            }
            adapter_index++;
        }
    }

    if ((dev_index < dev_mode_num) && (adapter_index == adapter_mode_num))
    {
        zx_memcpy(&merge_mode_list[mode_index], &dev_mode_list[dev_index], 
                   sizeof(CBiosModeInfoExt)*(dev_mode_num - dev_index));
        num_mode += (dev_mode_num - dev_index);
    }

    if(valid_adapter_mode_flag)
    {
        zx_free(valid_adapter_mode_flag);
        valid_adapter_mode_flag = NULL;
    }

    return num_mode;
}

int disp_cbios_cbmode_to_drmmode(disp_info_t *disp_info, int output, void* cbmode, int i, struct drm_display_mode *drm_mode)
{
    void                *pcbe = disp_info->cbios_ext;
    CBIOS_GET_MODE_TIMING_PARAM   get_timing = {0};
    CBIOS_TIMING_ATTRIB    timing_attrib = {0};
    PCBiosModeInfoExt  cbios_mode = (PCBiosModeInfoExt)cbmode + i;

    if(!cbios_mode || !drm_mode)
    {
        return  E_FAIL;
    }
    
    get_timing.DeviceId = output;
    get_timing.pMode = cbios_mode;
    get_timing.pTiming = &timing_attrib;
    if(CBIOS_OK != CBiosGetModeTiming(pcbe, &get_timing))
    {
        return  E_FAIL;
    }

    drm_mode->clock = timing_attrib.PixelClock / 10;
    drm_mode->hdisplay = timing_attrib.XRes;
    drm_mode->hsync_start = timing_attrib.HorSyncStart;
    drm_mode->hsync_end = timing_attrib.HorSyncEnd;
    drm_mode->htotal = timing_attrib.HorTotal;
    drm_mode->hskew = 0;

    drm_mode->vdisplay = timing_attrib.YRes;
    drm_mode->vsync_start = timing_attrib.VerSyncStart;
    drm_mode->vsync_end = timing_attrib.VerSyncEnd;
    drm_mode->vtotal = timing_attrib.VerTotal;
    drm_mode->vscan = 0;

#if DRM_VERSION_CODE < KERNEL_VERSION(5,9,0)
    drm_mode->vrefresh = timing_attrib.RefreshRate / 100;
#endif
    drm_mode->type |= DRM_MODE_TYPE_DRIVER;
    if (cbios_mode->isPreferredMode)
    {
        drm_mode->type |= DRM_MODE_TYPE_PREFERRED;
    }

    if (cbios_mode->InterlaceProgressiveCaps & 0x02)
    {
        drm_mode->flags |= DRM_MODE_FLAG_INTERLACE;
    }

    drm_mode->flags |= (timing_attrib.HVPolarity & HOR_NEGATIVE) ?
                DRM_MODE_FLAG_NHSYNC : DRM_MODE_FLAG_PHSYNC;
    drm_mode->flags |= (timing_attrib.HVPolarity & VER_NEGATIVE) ?
                DRM_MODE_FLAG_NVSYNC : DRM_MODE_FLAG_PVSYNC;
    drm_mode->flags |= DRM_MODE_FLAG_3D_NONE;

    return  S_OK;
}

int disp_cbios_3dmode_to_drmmode(disp_info_t *disp_info, int output, void* mode, int i, struct drm_display_mode *drm_mode)
{
    void                *pcbe = disp_info->cbios_ext;
    CBIOS_GET_MODE_TIMING_PARAM   get_timing = {0};
    CBIOS_TIMING_ATTRIB    timing_attrib = {0};
    CBiosModeInfoExt  cbios_mode = {0};    
    PCBIOS_3D_VIDEO_MODE_LIST  cb_3d_mode = (PCBIOS_3D_VIDEO_MODE_LIST)mode + i;

    if(!cb_3d_mode || !drm_mode)
    {
        return  E_FAIL;
    }

    cbios_mode.XRes = cb_3d_mode->XRes;
    cbios_mode.YRes = cb_3d_mode->YRes;
    cbios_mode.RefreshRate = cb_3d_mode->RefreshRate;
    cbios_mode.InterlaceProgressiveCaps = (cb_3d_mode->bIsInterlace)? 2 : 1;

    get_timing.DeviceId = output;
    get_timing.pMode = &cbios_mode;
    get_timing.pTiming = &timing_attrib;

    if(CBIOS_OK != CBiosGetModeTiming(pcbe, &get_timing))
    {
        return  E_FAIL;
    }

    drm_mode->clock = timing_attrib.PixelClock / 10;
    drm_mode->hdisplay = timing_attrib.XRes;
    drm_mode->hsync_start = timing_attrib.HorSyncStart;
    drm_mode->hsync_end = timing_attrib.HorSyncEnd;
    drm_mode->htotal = timing_attrib.HorTotal;
    drm_mode->hskew = 0;

    drm_mode->vdisplay = timing_attrib.YRes;
    drm_mode->vsync_start = timing_attrib.VerSyncStart;
    drm_mode->vsync_end = timing_attrib.VerSyncEnd;
    drm_mode->vtotal = timing_attrib.VerTotal;
    drm_mode->vscan = 0;

#if DRM_VERSION_CODE < KERNEL_VERSION(5,9,0)
    drm_mode->vrefresh = timing_attrib.RefreshRate / 100; 
#endif
    drm_mode->type |= DRM_MODE_TYPE_DRIVER;

    if (cb_3d_mode->bIsInterlace)
    {
        drm_mode->flags |= DRM_MODE_FLAG_INTERLACE;
    }
    drm_mode->flags |= (timing_attrib.HVPolarity & HOR_NEGATIVE) ?
             DRM_MODE_FLAG_NHSYNC : DRM_MODE_FLAG_PHSYNC;
    drm_mode->flags |= (timing_attrib.HVPolarity & VER_NEGATIVE) ?
             DRM_MODE_FLAG_NVSYNC : DRM_MODE_FLAG_PVSYNC;

    if (cb_3d_mode->SupportStructures.FramePacking)
    {
        drm_mode->flags |= DRM_MODE_FLAG_3D_FRAME_PACKING;
    }
    if (cb_3d_mode->SupportStructures.FieldAlternative)
    {
        drm_mode->flags |= DRM_MODE_FLAG_3D_FIELD_ALTERNATIVE;
    }
    if (cb_3d_mode->SupportStructures.LineAlternative)
    {
        drm_mode->flags |= DRM_MODE_FLAG_3D_LINE_ALTERNATIVE;
    }
    if (cb_3d_mode->SupportStructures.SideBySideFull)
    {
        drm_mode->flags |= DRM_MODE_FLAG_3D_SIDE_BY_SIDE_FULL;
    }
    if (cb_3d_mode->SupportStructures.LDepth)
    {
        drm_mode->flags |= DRM_MODE_FLAG_3D_L_DEPTH;
    }
    if (cb_3d_mode->SupportStructures.LDepthGraphics)
    {
        drm_mode->flags |= DRM_MODE_FLAG_3D_L_DEPTH_GFX_GFX_DEPTH;
    }
    if (cb_3d_mode->SupportStructures.TopAndBottom)
    {
        drm_mode->flags |= DRM_MODE_FLAG_3D_TOP_AND_BOTTOM;
    }
    if (cb_3d_mode->SupportStructures.SideBySideHalf)
    {
        drm_mode->flags |= DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF;
    }

    return  S_OK;
}

int disp_cbios_get_3dmode_size(disp_info_t* disp_info, int output)
{
    void                               *pcbe = disp_info->cbios_ext;
    CBIOS_MONITOR_3D_CAPABILITY_PARA   CbiosMonitor3DCapablitityPara    = {0};
    int  buf_size = 0, support = 0;

    CbiosMonitor3DCapablitityPara.DeviceId           = output;
    CbiosMonitor3DCapablitityPara.pMonitor3DModeList = NULL;

    if(CBIOS_OK == CBiosQueryMonitor3DCapability(pcbe, &CbiosMonitor3DCapablitityPara))
    {
        support = CbiosMonitor3DCapablitityPara.bIsSupport3DVideo;
    }

    if(support)
    {
        buf_size = CbiosMonitor3DCapablitityPara.Monitor3DModeNum;
        buf_size *= sizeof(CBIOS_3D_VIDEO_MODE_LIST);
    }

    return  buf_size;
}

int disp_cbios_get_3dmodes(disp_info_t *disp_info, int output, void* buffer, int buf_size)
{
    void                               *pcbe = disp_info->cbios_ext;
    CBIOS_MONITOR_3D_CAPABILITY_PARA   CbiosMonitor3DCapablitityPara    = {0};
    int  real_size = 0, real_num = 0;

    if(!buffer || !buf_size)
    {
        return real_num;
    }

    CbiosMonitor3DCapablitityPara.DeviceId           = output;
    CbiosMonitor3DCapablitityPara.pMonitor3DModeList = (PCBIOS_3D_VIDEO_MODE_LIST)buffer;

    CBiosQueryMonitor3DCapability(pcbe, &CbiosMonitor3DCapablitityPara);

    if(CbiosMonitor3DCapablitityPara.bIsSupport3DVideo)
    {
        real_size = CbiosMonitor3DCapablitityPara.Monitor3DModeNum * sizeof(CBIOS_3D_VIDEO_MODE_LIST);
        if(real_size > buf_size)
        {
            zx_error("OVERFLOW detected: malloc_size: %d, used size: %x.\n", buf_size, real_size);
        }
        real_num = CbiosMonitor3DCapablitityPara.Monitor3DModeNum;
    }

    return  real_num;
}

int disp_cbios_get_mode_timing(disp_info_t *disp_info, int output, struct drm_display_mode *drm_mode)
{
    void                *pcbe = disp_info->cbios_ext;
    CBIOS_GET_MODE_TIMING_PARAM   get_timing = {0};
    CBIOS_TIMING_ATTRIB    timing_attrib = {0};
    CBiosModeInfoExt  cbios_mode = {0};
    int temp = 0;

    if(!drm_mode)
    {
        return  E_FAIL;
    }

    cbios_mode.XRes = drm_mode->hdisplay;
    cbios_mode.YRes = drm_mode->vdisplay;
    temp = drm_mode->clock * 1000/drm_mode->htotal;
    cbios_mode.RefreshRate = temp * 100/drm_mode->vtotal;
    cbios_mode.InterlaceProgressiveCaps = (drm_mode->flags & DRM_MODE_FLAG_INTERLACE) ? 0x02 : 0x01;
    
    get_timing.DeviceId = output;
    get_timing.pMode = &cbios_mode;
    get_timing.pTiming = &timing_attrib;
    if(CBIOS_OK != CBiosGetModeTiming(pcbe, &get_timing))
    {
        return  E_FAIL;
    }

    drm_mode->crtc_clock = timing_attrib.PixelClock / 10;
    drm_mode->crtc_hdisplay = timing_attrib.XRes;
    drm_mode->crtc_hblank_start = timing_attrib.HorBStart;
    drm_mode->crtc_hblank_end = timing_attrib.HorBEnd;
    drm_mode->crtc_hsync_start = timing_attrib.HorSyncStart;
    drm_mode->crtc_hsync_end = timing_attrib.HorSyncEnd;
    drm_mode->crtc_htotal = timing_attrib.HorTotal;
    drm_mode->crtc_hskew = 0;

    drm_mode->crtc_vdisplay = timing_attrib.YRes;
    drm_mode->crtc_vblank_start = timing_attrib.VerBStart;
    drm_mode->crtc_vblank_end = timing_attrib.VerBEnd;
    drm_mode->crtc_vsync_start = timing_attrib.VerSyncStart;
    drm_mode->crtc_vsync_end = timing_attrib.VerSyncEnd;
    drm_mode->crtc_vtotal = timing_attrib.VerTotal;

    return S_OK;
}

void* disp_cbios_read_edid(disp_info_t *disp_info, int drv_dev)
{
    void         *pcbe    = disp_info->cbios_ext;

    CBIOS_PARAM_GET_EDID           cbParamGetEdid = {0};
    unsigned char                  *pEdid = NULL;

    pEdid = zx_calloc(EDID_BUF_SIZE);

    if(!pEdid)
    {
        return  NULL;
    }

    cbParamGetEdid.EdidBuffer    = pEdid;
    cbParamGetEdid.EdidBufferLen = EDID_BUF_SIZE;
    cbParamGetEdid.Size          = sizeof(CBIOS_PARAM_GET_EDID);
    cbParamGetEdid.DeviceId      = drv_dev;

    if(CBiosGetEdid(pcbe, &cbParamGetEdid) != CBIOS_OK)
    {
        zx_info("device id: 0x%x has no valid EDID.\n", drv_dev);
        zx_free(pEdid);
        pEdid = NULL;
    }

    return pEdid;
}

int disp_cbios_update_output_active(disp_info_t *disp_info, int* outputs)
{
    void                        *pcbe = disp_info->cbios_ext;
    CIP_ACTIVE_DEVICES          activeDevices = {0};

    zx_memcpy(activeDevices.DeviceId, outputs, sizeof(activeDevices.DeviceId));

    return (CBiosSetActiveDevice(pcbe, &activeDevices) == CBIOS_OK)? 0 : -1;
}

int disp_cbios_sync_vbios(disp_info_t *disp_info)
{
    int status = S_OK;
    CBIOS_VBIOS_DATA_PARAM DataParam = {0};

    if(CBIOS_OK != CBiosSyncDataWithVbios(disp_info->cbios_ext, &DataParam))
    {
        status = E_FAIL;
    }

    return status;
}

int disp_cbios_get_active_devices(disp_info_t *disp_info, int *devices)
{
    int status = S_OK;
    CIP_ACTIVE_DEVICES          activeDevices = {0};

    if(CBIOS_OK != CBiosGetActiveDevice(disp_info->cbios_ext, &activeDevices))
    {
        status = E_FAIL;
    }
    else
    {
        zx_memcpy(devices, activeDevices.DeviceId, sizeof(activeDevices.DeviceId));
    }

    return status;
}

int disp_cbios_detect_connected_output(disp_info_t *disp_info, int to_detect, int FullDetect)
{
    void                        *pcbe = disp_info->cbios_ext;
    CIP_DEVICES_DETECT_PARAM    devicesDetectParam = {0};
    int                         status;
    int                         curr_dev = 0;

    devicesDetectParam.DevicesToDetect = disp_info->support_output & to_detect;
    devicesDetectParam.FullDetect = FullDetect;

    status = CBiosDetectAttachedDisplayDevices(pcbe, &devicesDetectParam);

    if(status == CBIOS_OK)
    {
        curr_dev = devicesDetectParam.DetectedDevices;
    }

    return curr_dev;
}

//get monitor type per device id, connected = 0, supported monitor type, =1, current connected monitor type
int  disp_cbios_get_monitor_type(disp_info_t *disp_info, int device_id, int  connected)
{
    int  status = CBIOS_OK;
    int  monitor_type = 0;
    void* pcbe = disp_info->cbios_ext;
    CBIOS_QUERY_MONITOR_TYPE_PER_PORT   query_monitor = {0};

    if(device_id)
    {
        query_monitor.ActiveDevId = GET_LAST_BIT(device_id);
        query_monitor.bConnected = (connected)? CBIOS_TRUE : CBIOS_FALSE;

        status = CBiosGetMonitorTypePerPort(pcbe, &query_monitor);

        if(status == CBIOS_OK)
        {
            monitor_type = (int)query_monitor.MonitorType;
        }
    }
    monitor_type = disp_biosmonitor_to_output(monitor_type);
    
    return  monitor_type;
}

int disp_cbios_set_hdac_connect_status(disp_info_t *disp_info, int device , int bPresent, int bEldValid)
{
    void                    *pcbe         = disp_info->cbios_ext;
    CBIOS_HDAC_PARA         CbiosHDACPara = {0};
    int                     monitor_type = 0;
    monitor_type = disp_cbios_get_monitor_type(disp_info, device, FALSE);
    if (monitor_type & (UT_OUTPUT_TYPE_HDMI | UT_OUTPUT_TYPE_DP | UT_OUTPUT_TYPE_MHL))
    {
        CbiosHDACPara.Size = sizeof(CBIOS_HDAC_PARA);
        CbiosHDACPara.DeviceId = device;
        CbiosHDACPara.bPresent  = bPresent;
        CbiosHDACPara.bEldValid = bEldValid;

        CBiosSetHDACConnectStatus(pcbe, &CbiosHDACPara);
    }

    return S_OK;
}

static unsigned int disp_get_cbios_output_signal(zx_output_signal_t output_signal)
{
    unsigned int cb_output_signal = CBIOS_RGBOUTPUT;

    switch (output_signal)
    {
    case ZX_OUTPUT_SIGNAL_RGB:
        cb_output_signal = CBIOS_RGBOUTPUT;
        break;

    case ZX_OUTPUT_SIGNAL_YCBCR422:
        cb_output_signal = CBIOS_YCBCR422OUTPUT;
        break;

    case ZX_OUTPUT_SIGNAL_YCBCR444:
        cb_output_signal = CBIOS_YCBCR444OUTPUT;
        break;

    case ZX_OUTPUT_SIGNAL_YCBCR420:
        cb_output_signal = CBIOS_YCBCR420OUTPUT;
        break;

    default:
        cb_output_signal = CBIOS_RGBOUTPUT;
        zx_error("invalid output signal type %d, default to RGB");
        break;
    }

    return cb_output_signal;
}

void disp_get_hv_timing(PCBIOS_TIMING_ATTRIB pcbios_timing, struct drm_display_mode* mode)
{
    int temp = 0;
    pcbios_timing->Size = sizeof(CBIOS_TIMING_ATTRIB);
    pcbios_timing->FormatNum = 0;
    pcbios_timing->XRes = mode->crtc_hdisplay;
    pcbios_timing->YRes = mode->crtc_vdisplay;

    temp = mode->crtc_clock * 1000/mode->crtc_htotal;
    pcbios_timing->RefreshRate = temp * 100/mode->crtc_vtotal;
    pcbios_timing->PixelClock = mode->crtc_clock*10;
    pcbios_timing->AspectRatio = 0;
    pcbios_timing->HVPolarity = ((mode->flags & DRM_MODE_FLAG_NHSYNC)? HOR_NEGATIVE : HOR_POSITIVE)|
                                ((mode->flags & DRM_MODE_FLAG_NVSYNC)? VER_NEGATIVE : VER_POSITIVE);
    pcbios_timing->HorTotal = mode->crtc_htotal;
    pcbios_timing->HorDisEnd = mode->crtc_hblank_start;
    pcbios_timing->HorBStart = mode->crtc_hblank_start;
    pcbios_timing->HorBEnd = mode->crtc_hblank_end;
    pcbios_timing->HorSyncStart = mode->crtc_hsync_start;
    pcbios_timing->HorSyncEnd = mode->crtc_hsync_end;
    pcbios_timing->VerTotal = mode->crtc_vtotal;
    pcbios_timing->VerDisEnd = mode->crtc_vblank_start;
    pcbios_timing->VerBStart = mode->crtc_vblank_start;
    pcbios_timing->VerBEnd = mode->crtc_vblank_end;
    pcbios_timing->VerSyncStart = mode->crtc_vsync_start;
    pcbios_timing->VerSyncEnd = mode->crtc_vsync_end;
    pcbios_timing->PLLClock = 0;

}

int disp_cbios_set_mode(disp_info_t *disp_info, int crtc, struct drm_display_mode* mode, struct drm_display_mode* adjusted_mode, int flag)
{
    void*                   pcbe = disp_info->cbios_ext;
    CBIOS_STATUS            cb_status;
    int  temp = 0;

    CBiosSettingModeParams  mode_param = {0};

    temp = adjusted_mode->clock * 1000/adjusted_mode->htotal;
    mode_param.DestModeParams.RefreshRate = temp * 100/adjusted_mode->vtotal;

    mode_param.SourcModeParams.XRes = mode->hdisplay;
    mode_param.SourcModeParams.YRes = mode->vdisplay;
 
    mode_param.DestModeParams.XRes = adjusted_mode->hdisplay;
    mode_param.DestModeParams.YRes = adjusted_mode->vdisplay;
    
    mode_param.DestModeParams.InterlaceFlag = (adjusted_mode->flags & DRM_MODE_FLAG_INTERLACE)? 1 : 0;;
    mode_param.DestModeParams.AspectRatioFlag = 0;
    mode_param.DestModeParams.OutputSignal = disp_get_cbios_output_signal(ZX_OUTPUT_SIGNAL_RGB);

    mode_param.ScalerSizeParams.XRes = adjusted_mode->hdisplay;
    mode_param.ScalerSizeParams.YRes = adjusted_mode->vdisplay;

    if(flag & UES_CUSTOMIZED_TIMING_STRATEGY)
    {
        disp_get_hv_timing(&mode_param.SpecifyTiming.UserCustDestTiming.timing_para, mode);
        mode_param.SpecifyTiming.Flag = CBIOS_SPECIFY_TIMING_FLAG_USE_CUSTOMIZED_CRTC_PARA;
    }
    else
    {
        mode_param.SpecifyTiming.Flag = CBIOS_SPECIFY_TIMING_FLAG_DEFAULT;
    }

    mode_param.IGAIndex = crtc;
    mode_param.BitPerComponent = 8;
    mode_param.SkipIgaMode = (flag & UPDATE_CRTC_MODE_FLAG)? 0 : 1;
    mode_param.SkipDeviceMode = (flag & UPDATE_ENCODER_MODE_FLAG)? 0 : 1;

    if(mode_param.SkipIgaMode == 0)
    {
        zx_info("KMS set mode to path(iga_index-->active_device): %d-->0x%x.flag 0x%x\n", crtc, disp_info->active_output[crtc],flag);
    }

    if(adjusted_mode->flags & DRM_MODE_FLAG_3D_MASK)
    {
        mode_param.Is3DVideoMode = 1;
        mode_param.IsSingleBuffer= 0;
        if(mode->flags & DRM_MODE_FLAG_3D_FRAME_PACKING)
        {
            mode_param.Video3DStruct = FRAME_PACKING;
        }
        else if(mode->flags & DRM_MODE_FLAG_3D_FIELD_ALTERNATIVE)
        {
            mode_param.Video3DStruct = FIELD_ALTERNATIVE;
        }
        else if(mode->flags & DRM_MODE_FLAG_3D_LINE_ALTERNATIVE)
        {
            mode_param.Video3DStruct = LINE_ALTERNATIVE;
        }
        else if(mode->flags & DRM_MODE_FLAG_3D_SIDE_BY_SIDE_FULL)
        {
            mode_param.Video3DStruct = SIDE_BY_SIDE_FULL;
        }
        else if(mode->flags & DRM_MODE_FLAG_3D_L_DEPTH)
        {
            mode_param.Video3DStruct = L_DEPTH;
        }
        else if(mode->flags & DRM_MODE_FLAG_3D_L_DEPTH_GFX_GFX_DEPTH)
        {
            mode_param.Video3DStruct = L_DEPTH_GRAPHICS;
        }
        else if(mode->flags & DRM_MODE_FLAG_3D_TOP_AND_BOTTOM)
        {
            mode_param.Video3DStruct = TOP_AND_BOTTOM;
        }
        else if(mode->flags & DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF)
        {
            mode_param.Video3DStruct = SIDE_BY_SIDE_HALF;
        }
    }

    cb_status = CBiosSetModeToIGA(pcbe, &mode_param);

    return (cb_status == CBIOS_OK) ? S_OK : E_FAIL;
}

int disp_cbios_turn_onoff_screen(disp_info_t *disp_info, int iga, int bOn)
{
    void                        *pcbe = disp_info->cbios_ext;
    int                         cb_status;

    cb_status = CBiosSetIgaScreenOnOffState(pcbe, bOn, iga);

    return (cb_status == CBIOS_OK) ? S_OK : E_FAIL;
}

int disp_cbios_set_dpms(disp_info_t *disp_info, int device, int dpms_mode, unsigned int flags)
{
    int status = S_OK;

    if(CBIOS_OK != CBiosSetDisplayDevicePowerState(disp_info->cbios_ext, device, dpms_mode, flags))
    {
        status = E_FAIL;
    }
    
    return status;
}

int disp_cbios_set_gamma(disp_info_t *disp_info, int pipe, void* data)
{
    int  status;
    void  *pcbe = disp_info->cbios_ext;
    CBIOS_GAMMA_PARA gamma_para = {0};
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    struct  drm_color_lut* ori_lut = (struct  drm_color_lut*)data;
    int* gamma = NULL;
    int i = 0;
#endif
    
    if(pipe >= disp_info->num_crtc)
    {
        return E_FAIL;
    }

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    gamma = zx_calloc(sizeof(int) * 256);
    if(ori_lut)
    {
        for(i = 0; i < 256; i++)
        {
            gamma[i] = ((ori_lut[i].blue >> 8) & 0xFF) + (ori_lut[i].green & 0xFF00) + ((ori_lut[i].red << 8) & 0xFF0000); 
        }
    }
    else
    {
        for(i = 0; i < 256; i++)
        {
            gamma[i] = i + (i << 8) + (i << 16); 
        }
    }
    gamma_para.pGammaTable = gamma;
#else
    gamma_para.pGammaTable = data;
#endif

    gamma_para.IGAIndex = pipe;
    gamma_para.Device = disp_info->active_output[pipe];
    gamma_para.StreamType = 1 << CBIOS_STREAM_PS;
    gamma_para.FisrtEntryIndex = 0;
    gamma_para.EntryNum = 256;
    gamma_para.Flags.bConfigGamma = 1;
    gamma_para.Flags.bSetLUT = 1;
    gamma_para.LutMode = CBIOS_GAMMA_8BIT_SINGLE;

    status = CBiosSetGamma(pcbe, &gamma_para);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    if(gamma)
    {
        zx_free(gamma);
        gamma = NULL;
    }
#endif

    return (status == CBIOS_OK)? S_OK : E_FAIL;
}

/*
int disp_mode_fixup(disp_info_t *disp_info, zx_mode_info_t *mode)
{
    output_t         *output = dispmgri_get_output_by_type(disp_info, mode->output);
    zx_mode_t       *modes  = NULL;

    unsigned int    caps    = 0;
    int             delta_x = 0;
    int             delta_y = 0;
    unsigned int    delta_xy= -1;

    int i, status = E_FAIL;

    if(output == NULL)
    {
        return status;
    }

    modes = output->modes;

    if(mode->bpp == 8)
    {
        caps = 0x02;
    }
    else if(mode->bpp == 16)
    {
        caps = 0x04;
    }
    else if(mode->bpp == 32)
    {
        caps = 0x01;
    }

    for(i = 0; i < output->num_modes; i++)
    {
        if((modes[i].xres == mode->width) &&
           (modes[i].yres == mode->height) &&
           (modes[i].color_depth_caps & caps))
        {
            mode->dst_width   = modes[i].xres;
            mode->dst_height  = modes[i].yres;
            mode->dst_refresh = modes[i].refresh_rate;
            break;
        }

        delta_x = modes[i].xres - mode->width;
        delta_y = modes[i].yres - mode->height;

        if((delta_x >= 0) &&
           (delta_y >= 0) &&
           (delta_xy > delta_x + delta_y) &&
           (modes[i].color_depth_caps & caps))
        {
            mode->dst_width   = modes[i].xres;
            mode->dst_height  = modes[i].yres;
            mode->dst_refresh = modes[i].refresh_rate;
            delta_xy = delta_x + delta_y;
        }
    }

    status = mode->dst_refresh ? S_OK:E_FAIL;

    return status;

} 

int disp_pwm_func_ctrl(disp_info_t *disp_info, zx_pwm_func_ctrl_t *pwm)
{
    output_t *output = NULL;
    void     *pvcbe  = disp_info->cbios_ext;

    CBIOS_PWM_FUNCTION_CTRL_PARAMS pwm_param = {0};

    int status = E_FAIL;

    switch(pwm->type)
    {
    case ZX_SET_FAN:
        pwm_param.PWMFunction = PWM_SET_FAN_CTRL_STATUS;
        pwm_param.PWMFuncSetting.FanCtrlArg.byFanSpeedLevel = pwm->value;

        break;

    case ZX_SET_BACKLIGHT:
        pwm_param.PWMFunction = PWM_SET_BACKLIGHT_STATUS;
        pwm_param.PWMFuncSetting.BLctrlArg.byBLLevel = pwm->value;

        output = disp_get_output_by_type(disp_info, pwm->output);

        output->curr_back_light_level = pwm->value;

        break;

    case ZX_GET_FAN:
        pwm_param.PWMFunction = PWM_GET_FAN_CTRL_STATUS;

        break;

    case ZX_GET_BACKLIGHT:

        pwm_param.PWMFunction = PWM_GET_BACKLIGHT_STATUS;
        break;

    default:
        return E_FAIL;
    }

    zx_mutex_lock(disp_mgr->cbios_lock);
    status = CBiosPWMFunctionCtrl(pvcbe, &pwm_param);
    zx_mutex_unlock(disp_mgr->cbios_lock);

    if(status != 0)
    {
        zx_error("pwm setting fail\n");
        status = E_FAIL;
    }

    return status;
} */

int  disp_cbios_dbg_level_get(disp_info_t *disp_info)
{
    CBIOS_DBG_LEVEL_CTRL  dbg_level_ctl = {0};

    dbg_level_ctl.Size = sizeof(CBIOS_DBG_LEVEL_CTRL);
    dbg_level_ctl.bGetValue = 1;

    CBiosDbgLevelCtl(&dbg_level_ctl);

    return  dbg_level_ctl.DbgLevel;
}

void  disp_cbios_dbg_level_set(disp_info_t *disp_info, int dbg_level)
{
    CBIOS_DBG_LEVEL_CTRL  dbg_level_ctl = {0};
    dbg_level_ctl.Size = sizeof(CBIOS_DBG_LEVEL_CTRL);
    dbg_level_ctl.DbgLevel = dbg_level;
    
    CBiosDbgLevelCtl(&dbg_level_ctl);
}

int  disp_cbios_get_output_name(disp_info_t *disp_info,  int  type)
{
    void*  pcbe = disp_info->cbios_ext;
    CBIOS_STATUS   status = CBIOS_OK;
    CBIOS_GET_DEVICE_NAME   GetDevName = {0};

    GetDevName.Size = sizeof(CBIOS_GET_DEVICE_NAME);
    GetDevName.DeviceId = type;

    status = CBiosGetDeviceName(pcbe, &GetDevName);

    if(status == CBIOS_OK)
    {
        //zx_strncpy(output->name, GetDevName.pDeviceName, sizeof(output->name));
    }

    return  (status == CBIOS_OK)? S_OK : E_FAIL;
}

int disp_cbios_get_connector_attrib(disp_info_t *disp_info, zx_connector_t *zx_connector)
{
    void*  pcbe = disp_info->cbios_ext;
    CBIOS_STATUS   status = CBIOS_OK;
    CBiosMonitorAttribute attrib = {0};

    attrib.Size = sizeof(CBiosMonitorAttribute);
    attrib.ActiveDevId = zx_connector->output_type;

    status = CBiosQueryMonitorAttribute(pcbe, &attrib);

    if (status == CBIOS_OK)
    {
        zx_connector->base_connector.display_info.width_mm = attrib.MonitorHorSize;
        zx_connector->base_connector.display_info.height_mm = attrib.MonitorVerSize;
        zx_connector->monitor_type = disp_biosmonitor_to_output(attrib.MonitorType);
        zx_connector->support_audio = attrib.bSupportHDAudio ? 1 : 0;
    }
    
    return  (status == CBIOS_OK)? S_OK : E_FAIL;
}

int  disp_cbios_get_crtc_mask(disp_info_t *disp_info,  int device)
{
    void*  pcbe = disp_info->cbios_ext;
    CBIOS_STATUS   status = CBIOS_OK;
    CBIOS_GET_IGA_MASK   GetIgaMask = {0};

    GetIgaMask.Size = sizeof(CBIOS_GET_IGA_MASK);
    GetIgaMask.DeviceId = device;

    status = CBiosGetIgaMask(pcbe, &GetIgaMask);

    return  (status == CBIOS_OK)?  GetIgaMask.IgaMask : 0;
}

int disp_cbios_get_clock(disp_info_t *disp_info, unsigned int type, unsigned int *output)
{
    CBios_GetClock_Params GetClock = {0};

    int   status = CBIOS_OK;

    GetClock.Size      = sizeof(CBios_GetClock_Params);
    GetClock.ClockFreq = output;

    if(type == ZX_QUERY_ENGINE_CLOCK)
    {
        GetClock.ClockType = CBIOS_ECLKTYPE;
    }
    else if(type == ZX_QUERY_I_CLOCK)
    {
        GetClock.ClockType = CBIOS_ICLKTYPE;
    }
    else if (type == ZX_QUERY_CPU_FREQUENCE)
    {
       GetClock.ClockType = CBIOS_CPUFRQTYPE;
    }
    else
    {
        zx_error("unknown get clock type: %d.\n", type);

        zx_assert(0);
    }

    status = CBiosGetClock(disp_info->cbios_ext, &GetClock);

    if(status != CBIOS_OK)
    {
        zx_error("dispmgri_get_clock failed: %d.\n", status);
    }

    return (status == CBIOS_OK) ? S_OK : E_FAIL;
}

int disp_cbios_set_clock(disp_info_t *disp_info, unsigned int type, unsigned int para)
{
    CBios_SetClock_Params SetClock = {0};

    int   status = CBIOS_OK;

    SetClock.Size = sizeof(CBios_SetClock_Params);

    if (type == ZX_SET_I_CLOCK)
    {
        SetClock.ClockType = CBIOS_ICLKTYPE;
        SetClock.ClockFreq = para;
    }
    else if (type == ZX_SET_CPU_FREQUENCE)
    {
        SetClock.ClockType = CBIOS_CPUFRQTYPE;
        SetClock.ClockFreq = para;
    }
    else
    {
        zx_error("unknown set clock type: %d.\n", type);

        zx_assert(0);
    }

    status = CBiosSetClock(disp_info->cbios_ext, &SetClock);

    if(status != CBIOS_OK)
    {
        zx_error("dispmgri_set_clock failed: %d.\n", status);
    }

    return (status == CBIOS_OK) ? S_OK : E_FAIL;
}

int disp_cbios_enable_hdcp(disp_info_t *disp_info, unsigned int enable, unsigned int devices)
{
    CBiosContentProtectionOnOffParams hdcp_para = {0};

    int   status = CBIOS_OK;

    hdcp_para.Size = sizeof(CBiosContentProtectionOnOffParams);
    hdcp_para.DevicesId = devices;
    hdcp_para.bHdcpStatus = enable ? CBIOS_TRUE : CBIOS_FALSE;

    status = CBiosContentProtectionOnOff(disp_info->cbios_ext, &hdcp_para);

    if (status != CBIOS_OK)
    {
        zx_error("dispmgri_cbios_enable_dhcp failed\n");
    }

    return (status == CBIOS_OK) ? S_OK : E_FAIL;
}

int disp_cbios_get_hdcp_status(disp_info_t *disp_info, zx_hdcp_op_t *dhcp_op, unsigned int devices)
{
    CBIOS_HDCP_STATUS_PARA     hdcp_status = {0};

    int status = CBIOS_OK;

    hdcp_status.Size = sizeof(CBIOS_HDCP_STATUS_PARA);
    hdcp_status.DevicesId = devices;

    status = CBiosGetHDCPStatus(disp_info->cbios_ext, &hdcp_status);

    if (status == CBIOS_OK)
    {
        dhcp_op->result = hdcp_status.HdcpStatus == CBIOS_TRUE ? ZX_HDCP_ENABLED : ZX_HDCP_FAILED;
    }
    else
    {
        zx_error("dispmgri_cbios_get_hdcp_status failed\n");
    }
    
    return (status == CBIOS_OK) ? S_OK : E_FAIL;
}

static unsigned int DrmFormat2CBiosFormat(unsigned int drm_format)
{
    CBIOS_FORMAT cbios_format = CBIOS_FMT_INVALID;
    switch (drm_format)
    {
        case DRM_FORMAT_RGB565:
            cbios_format = CBIOS_FMT_R5G6B5;
            break;
        case DRM_FORMAT_ARGB8888:
            cbios_format = CBIOS_FMT_A8R8G8B8;
            break;
        case DRM_FORMAT_ABGR8888:
            cbios_format = CBIOS_FMT_A8B8G8R8;
            break;
        case DRM_FORMAT_XBGR8888:
            cbios_format = CBIOS_FMT_X8B8G8R8;
            break;
        case DRM_FORMAT_XRGB8888:
            cbios_format = CBIOS_FMT_X8R8G8B8;
            break;
        case DRM_FORMAT_ARGB2101010:
            cbios_format = CBIOS_FMT_A2R10G10B10;
            break;
        case DRM_FORMAT_YUYV:
        case DRM_FORMAT_YVYU:
            cbios_format = CBIOS_FMT_CRYCBY422_16BIT;
            break;
        case DRM_FORMAT_UYVY:
        case DRM_FORMAT_VYUY:
            cbios_format = CBIOS_FMT_YCRYCB422_16BIT;
            break;
        case DRM_FORMAT_AYUV:
            cbios_format = CBIOS_FMT_YCBCR8888_32BIT;
            break;
        default:
            cbios_format = CBIOS_FMT_A8R8G8B8;
            break;
    }

    return cbios_format;
}

int disp_cbios_crtc_flip(disp_info_t *disp_info, zx_crtc_flip_t *arg)
{
    struct drm_framebuffer *fb = arg->fb;
    struct drm_zx_framebuffer *zxfb = fb? to_zxfb(arg->fb) : NULL;
    CBIOS_SURFACE_ATTRIB  stream_attr = {0};
    CBIOS_WINDOW_PARA  src_win = {0};
    CBIOS_WINDOW_PARA  dst_win = {0};
    CBIOS_OVERLAY_INFO   overlay = {0};
    CBIOS_STREAM_PARA   stream_para = {0};
    CBIOS_UPDATE_FRAME_PARA  update_frame = {0};

    update_frame.Size = sizeof(update_frame);
    update_frame.IGAIndex = arg->crtc;
    update_frame.pStreamPara[arg->stream_type] = &stream_para;

    stream_para.StreamType = arg->stream_type;
    if(fb)
    {
        stream_para.FlipMode = CBIOS_STREAM_FLIP_WITH_ENABLE;
    }
    else
    {
        stream_para.FlipMode = CBIOS_STREAM_FLIP_WITH_DISABLE;
    }

    if(fb)
    {
        trace_zxgfx_crtc_flip(arg->crtc, arg->stream_type, zxfb->obj);

        stream_para.pSurfaceAttrib = &stream_attr;
        stream_para.pSrcWindow = &src_win;
        stream_para.pDispWindow = &dst_win;

        stream_attr.StartAddr = zxfb->obj->info.gpu_virt_addr;
        stream_attr.SurfaceSize = (fb->width) | (fb->height << 16);
#if DRM_VERSION_CODE < KERNEL_VERSION(4, 11, 0)
        stream_attr.SurfaceFmt = DrmFormat2CBiosFormat(fb->pixel_format);
#else        
        stream_attr.SurfaceFmt = DrmFormat2CBiosFormat(fb->format->format);
#endif        
        stream_attr.Pitch = fb->pitches[0];
        stream_attr.bCompress = (zxfb->obj->info.compress_format != 0);
        stream_attr.bTiled = zxfb->obj->info.tiled;
        if(stream_attr.bTiled)
        {
            stream_attr.WidthInTile = zxfb->obj->info.tiled_width;
        }

        src_win.Position = arg->src_x | (arg->src_y << 16);
        src_win.WinSize = arg->src_w | (arg->src_h << 16);

        dst_win.Position = arg->crtc_x | (arg->crtc_y << 16);
        dst_win.WinSize = arg->crtc_w | (arg->crtc_h << 16);

#if  DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
        if(stream_para.StreamType != CBIOS_STREAM_PS)
        {
            stream_para.pOverlayInfo = &overlay;
            if(arg->blend_mode == DRM_MODE_BLEND_PIXEL_NONE)
            {
                overlay.KeyMode = CBIOS_CONSTANT_ALPHA;
                overlay.ConstantAlphaBlending.ConstantAlpha = (CBIOS_U8)(arg->const_alpha >> 8);
            }
            else if(arg->blend_mode == DRM_MODE_BLEND_COVERAGE)  //coverage with plane alpha
            {
                overlay.KeyMode = CBIOS_ALPHA_BLENDING;
                overlay.AlphaBlending.bSsTsAlphaBlending = (arg->blend_alpha_source)? 0 : 1;
                overlay.AlphaBlending.bBlendingMode = 0;
                overlay.AlphaBlending.bUsePlaneAlpha = 1;
                overlay.AlphaBlending.dwPlaneValue = (CBIOS_U8)(arg->plane_alpha >> 8);
                if(overlay.AlphaBlending.bSsTsAlphaBlending == 0)
                {
                    overlay.AlphaBlending.bInvertAlpha = 1;
                }
            }
            else  //premultied with plane alpha
            {
                overlay.KeyMode = CBIOS_ALPHA_BLENDING;
                overlay.AlphaBlending.bSsTsAlphaBlending = (arg->blend_alpha_source)? 0 : 1;
                overlay.AlphaBlending.bBlendingMode = 1;
                overlay.AlphaBlending.bUsePlaneAlpha = 1;
                overlay.AlphaBlending.dwPlaneValue = (CBIOS_U8)(arg->plane_alpha >> 8);
            }
        }
#endif
    }

    return (CBiosUpdateFrame(disp_info->cbios_ext, &update_frame) == CBIOS_OK) ? 0 : -1;
}


int disp_cbios_update_cursor(disp_info_t *disp_info, zx_cursor_update_t *arg)
{
    struct drm_framebuffer *fb = NULL;
    struct drm_zx_framebuffer *zxfb = NULL;
    CBIOS_CURSOR_PARA  update_cursor = {0};

    update_cursor.bVsyncOn = arg->vsync_on;
    if (arg->bo)
    {
        update_cursor.bDisable = 0;
        if (arg->width == 64 && arg->height == 64)
        {
            update_cursor.CursorSize = CBIOS_CURSOR_SIZE_64x64;
        }
        else if (arg->width == 128 && arg->height == 128)
        {
            update_cursor.CursorSize = CBIOS_CURSOR_SIZE_128x128;
        }
        update_cursor.StartAddr = arg->bo->info.gpu_virt_addr;
        update_cursor.PosX = arg->pos_x;
        update_cursor.PosY = arg->pos_y;
    }
    else
    {
        update_cursor.bDisable = 1;
    }

    update_cursor.Size = sizeof(update_cursor);
    update_cursor.IGAIndex = arg->crtc;
    update_cursor.Type = CBIOS_COVERAGE_CURSOR;

    return  (CBiosSetCursor(disp_info->cbios_ext, &update_cursor) == CBIOS_OK)? 0 : -1;
}

int disp_cbios_interrupt_enable_disable(disp_info_t *disp_info, int enable, unsigned int int_type)
{
    int status = CBIOS_OK;
    CBIOS_INT_ENABLE_DISABLE_PARA int_para = {0};

    int_para.bEnableInt = enable;
    int_para.IntFlags = 0;
    int_para.AdvancedIntFlags = 0;

    if(int_type == 0xFFFFFFFF)
    {
        int_para.bUpdAllInt = 1;
        int_para.bUpdAllAdvInt = 1;
    }
    else
    {
        if (int_type & INT_HDMI_CP)
        {
            int_para.IntFlags |= CBIOS_HDCP_INT;
        }
        if (int_type & INT_HDCODEC)
        {
            int_para.IntFlags |= CBIOS_HDA_CODEC_INT;
        }
        if (int_type & INT_HOTPLUG)
        {
            int_para.IntFlags |= (CBIOS_DP_1_INT | CBIOS_DP_2_INT | CBIOS_DVO_INT);
        }
        if (int_type & INT_VSYNC_1)
        {
            int_para.IntFlags |= CBIOS_VSYNC_1_INT;
        }
        if (int_type & INT_VSYNC_2)
        {
            int_para.IntFlags |= CBIOS_VSYNC_2_INT;
        }
        if (int_type & INT_VSYNC_3)
        {
            int_para.IntFlags |= CBIOS_VSYNC_3_INT;
        }
        if (int_type & INT_CEC_CONTROL)
        {
            int_para.IntFlags |= CBIOS_CEC_INT;
        }
        if (int_type & INT_FENCE)
        {   //0x40C
            int_para.AdvancedIntFlags |= CBIOS_FENCE_INT;
            int_para.AdvancedIntFlags |= CBIOS_PAGE_FAULT_INT;
            int_para.AdvancedIntFlags |= CBIOS_MXU_INVALID_ADDR_FAULT_INT;
        }
    }
    
    status = CBiosInterruptEnableDisable(disp_info->cbios_ext, &int_para);

    return (status == CBIOS_OK) ? S_OK : E_FAIL;
}

int disp_cbios_get_interrupt_info(disp_info_t *disp_info, unsigned int *interrupt_mask)
{
    int status = CBIOS_OK;
    CBIOS_INTERRUPT_INFO interrupt_info = {0};

    status = CBiosGetInterruptInfo(disp_info->cbios_ext, &interrupt_info);

    if(interrupt_info.InterruptType & CBIOS_VSYNC_1_INT)
    {            
        *interrupt_mask |= INT_VSYNC_1;
    }

    if(interrupt_info.InterruptType & CBIOS_VSYNC_2_INT)
    {            
        *interrupt_mask |= INT_VSYNC_2;
    }

    if(interrupt_info.InterruptType & CBIOS_VSYNC_3_INT)
    {            
        *interrupt_mask |= INT_VSYNC_3;
    }

    if(interrupt_info.InterruptType & (CBIOS_DP_1_INT | CBIOS_DP_2_INT))
    {            
        *interrupt_mask |= INT_HOTPLUG;
    }

    if(interrupt_info.InterruptType & CBIOS_HDCP_INT)
    {
        *interrupt_mask |= INT_HDMI_CP;
    }

    if(interrupt_info.InterruptType & CBIOS_CEC_INT)
    {
        *interrupt_mask |= INT_CEC_CONTROL;
    }

    if(interrupt_info.InterruptType & CBIOS_HDA_CODEC_INT)
    {
        *interrupt_mask |= INT_HDCODEC;
    }

    if((interrupt_info.AdvancedIntType & CBIOS_FENCE_INT) ||
        (interrupt_info.AdvancedIntType & CBIOS_PAGE_FAULT_INT) ||
        (interrupt_info.AdvancedIntType & CBIOS_MXU_INVALID_ADDR_FAULT_INT))
    {
        *interrupt_mask |= INT_FENCE;
    }

    return (status == CBIOS_OK) ? S_OK : E_FAIL;
}

int   disp_cbios_get_dpint_type(disp_info_t *disp_info,unsigned int device)
{
    CBIOS_DP_INT_PARA DpIntPara = {0};
    int hpd = DP_HPD_NONE;

    DpIntPara.Size = sizeof(CBIOS_DP_INT_PARA);
    DpIntPara.DeviceId = device;
    if(CBIOS_OK == CBiosGetDPIntType(disp_info->cbios_ext, &DpIntPara))
    {
        hpd = DpIntPara.IntType;
    }

    return hpd;
}

int  disp_cbios_handle_dp_irq(disp_info_t *disp_info, unsigned int device, int int_type, int* need_detect, int* need_comp_edid)
{
    CBIOS_DP_HANDLE_IRQ_PARA  handle_irq = {0};
    int  status = E_FAIL;

    handle_irq.Size = sizeof(CBIOS_DP_HANDLE_IRQ_PARA);
    handle_irq.DeviceId = device;
    handle_irq.IntType = int_type;
    if(CBIOS_OK == CBiosHandleDPIrq(disp_info->cbios_ext, &handle_irq))
    {
        status = S_OK;
        if(need_detect)
        {
            *need_detect = handle_irq.bNeedDetect;
        }
        
        if(need_comp_edid)
        {
            *need_comp_edid = handle_irq.bNeedCompEdid;
        }
    }

    return status;
}


void disp_cbios_dump_registers(disp_info_t *disp_info, int type)
{
    CBIOS_DUMP_PARA para = {0};

    para.Size = sizeof(CBIOS_DUMP_PARA);

    para.DumpType = CBIOS_DUMP_VCP_INFO  | 
                    CBIOS_DUMP_MODE_INFO |
                    CBIOS_DUMP_CLOCK_INFO;

    if(type &  DUMP_REGISTER_STREAM)
    {
        para.DumpType |= CBIOS_DUMP_PS1_REG |
                         CBIOS_DUMP_PS2_REG |
                         CBIOS_DUMP_SS1_REG |
                         CBIOS_DUMP_SS2_REG |
                         CBIOS_DUMP_TS1_REG |
                         CBIOS_DUMP_TS2_REG;
    }

    CBiosDumpInfo(disp_info->cbios_ext, &para);
}

int disp_cbios_set_hda_codec(disp_info_t *disp_info, zx_connector_t*  zx_connector)
{
    void                        *pcbe = disp_info->cbios_ext;
    CBIOS_HDAC_PARA             CbiosHDACPara = {0};
    int                         cb_status = CBIOS_OK;

    if((zx_connector->monitor_type == UT_OUTPUT_TYPE_HDMI) ||
       (zx_connector->monitor_type == UT_OUTPUT_TYPE_MHL) ||
       (zx_connector->monitor_type == UT_OUTPUT_TYPE_DP)) 
    {
        CbiosHDACPara.Size      = sizeof(CBIOS_HDAC_PARA);
        CbiosHDACPara.DeviceId = zx_connector->output_type;

        cb_status = CBiosSetHDACodecPara(pcbe, &CbiosHDACPara);
    }
    return (cb_status == CBIOS_OK) ?  S_OK : E_FAIL;
}

int disp_cbios_get_hdmi_audio_format(disp_info_t *disp_info, unsigned int device_id, zx_hdmi_audio_formats *audio_formats)
{
    void                          *pcbe            = disp_info->cbios_ext;

    unsigned int                  buffer_size      = 0;
    unsigned int                  real_buffer_size = 0;
    unsigned int                  num_formats      = 0;
    unsigned int                  i                = 0;
    CBiosHDMIAudioFormat         *cb_audio_formats = NULL;
    CBIOS_STATUS                  cb_status        = CBIOS_OK;

    cb_status = CBiosGetHDMIAudioFomatListBufferSize(pcbe, device_id, &buffer_size);
    if ((cb_status != CBIOS_OK) || (buffer_size == 0))
    {
        zx_error("can not get the audio format buffer size\n");
        return E_FAIL;
    }

    cb_audio_formats = zx_calloc(buffer_size);
    if (cb_audio_formats == NULL)
    {
        return E_FAIL;
    }

    num_formats = buffer_size / sizeof(CBiosHDMIAudioFormat);
    for(i = 0; i < num_formats; i++)
    {
       cb_audio_formats[i].Size = sizeof(CBiosHDMIAudioFormat);
    }

    cb_status = CBiosGetHDMIAudioFomatList(pcbe, device_id, cb_audio_formats, &real_buffer_size);

    if ((cb_status != CBIOS_OK) || (real_buffer_size == 0))
    {
        zx_error("can not get the audio format\n");
        return E_FAIL;
    }

    if (num_formats > sizeof(audio_formats->audio_formats)/sizeof(zx_hdmi_audio_format_t))
    {
        zx_error("the num of audio formats exceeds the buffer size, so cut it.");
        num_formats = sizeof(audio_formats->audio_formats)/sizeof(zx_hdmi_audio_format_t);
    }

    audio_formats->num_formats = num_formats;
    for (i = 0; i < num_formats; i++)
    {
        audio_formats->audio_formats[i].format = (ZX_HDMI_AUDIO_FORMAT_TYPE)cb_audio_formats[i].Format;
        audio_formats->audio_formats[i].max_channel_num= cb_audio_formats[i].MaxChannelNum;
        audio_formats->audio_formats[i].sample_rate_unit = cb_audio_formats[i].SampleRateUnit;
        audio_formats->audio_formats[i].unit = cb_audio_formats[i].Unit;
    }

    if (cb_audio_formats)
    {
        zx_free(cb_audio_formats);
        cb_audio_formats = NULL;
    }
    return S_OK;
}

void disp_cbios_reset_hw_block(disp_info_t *disp_info, zx_hw_block hw_block)
{
    CBIOS_HW_BLOCK  cbios_hw_block = {0};

    if(hw_block == ZX_HW_IGA)
    {
        cbios_hw_block = CBIOS_HW_IGA;
    }
    else
    {
        cbios_hw_block = CBIOS_HW_NONE;
    }

    CBiosResetHWBlock(disp_info->cbios_ext, cbios_hw_block);
}

int disp_cbios_get_counter(disp_info_t* disp_info, zx_get_counter_t* get_counter)
{
    CBIOS_GET_HW_COUNTER   CbiosGetHwCnt = {0};
    int  status = E_FAIL;

    if(!get_counter || get_counter->crtc_index >= disp_info->num_crtc)
    {
        return  status;
    }

    CbiosGetHwCnt.IgaIndex = get_counter->crtc_index;
    
   if(get_counter->hpos)
   {
       CbiosGetHwCnt.bGetPixelCnt = 1;
   }
   
   if(get_counter->vpos)
   {
       CbiosGetHwCnt.bGetLineCnt = 1;
   }
   
   if(get_counter->vblk)
   {
       CbiosGetHwCnt.bGetFrameCnt = 1;
   }

   if(CBIOS_OK == CBiosGetHwCounter(disp_info->cbios_ext, &CbiosGetHwCnt))
   {
       status = S_OK;
       if(get_counter->hpos)
       {
           *get_counter->hpos = CbiosGetHwCnt.Value[CBIOS_COUNTER_PIXEL];
       }
       if(get_counter->vpos)
       {
           *get_counter->vpos = CbiosGetHwCnt.Value[CBIOS_COUNTER_LINE];
       }
       if(get_counter->vblk)
       {
           *get_counter->vblk = CbiosGetHwCnt.Value[CBIOS_COUNTER_FRAME];
       }
       if(get_counter->in_vblk)
       {
           *get_counter->in_vblk = CbiosGetHwCnt.bInVblank;
       }
   }

   return status;
}

void  disp_cbios_brightness_set(disp_info_t* disp_info, unsigned int brightness)
{
    void                        *pcbe = disp_info->cbios_ext;
    int  status = CBIOS_OK;
    int  bEDPLighted = 0, device_id = 0;
    CBIOS_BACKLIGHT_PARA  blPara = {0};
    CBiosMonitorAttribute  monitorAttr = {0};
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device*  drm = zx_card->drm_dev;
    struct drm_connector* connector = NULL;
    zx_connector_t*  zx_connector = NULL;
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    struct drm_connector_list_iter conn_iter;
#endif

    mutex_lock(&drm->mode_config.mutex);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_begin(drm, &conn_iter);
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    drm_for_each_connector_iter(connector, &conn_iter)
#else

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    drm_for_each_connector(connector, drm)
#else
    list_for_each_entry(connector, &drm->mode_config.connector_list, head)
#endif    

#endif
    {
        zx_connector = to_zx_connector(connector);
        if(zx_connector->monitor_type == UT_OUTPUT_TYPE_PANEL)
        {
            bEDPLighted = 1;
            device_id = zx_connector->output_type;
            break;
        }
    }
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_end(&conn_iter);
#endif

    mutex_unlock(&drm->mode_config.mutex);

//till now, only edp support backlight adjust
    if(bEDPLighted)
    {
        monitorAttr.Size = sizeof(monitorAttr);
        monitorAttr.ActiveDevId = device_id;

        CBiosQueryMonitorAttribute(pcbe, &monitorAttr);

        if(monitorAttr.bSupportBLCtrl)
        {
            blPara.Size = sizeof(CBIOS_BACKLIGHT_PARA);
            blPara.BacklightValue = brightness;
            blPara.DeviceId = device_id;

            status = CBiosSetBacklight(pcbe, &blPara);
        }
    }

//store brightness
    disp_info->is_brightness_valid = 1;
    disp_info->brightness = brightness;
}

unsigned int  disp_cbios_brightness_get(disp_info_t* disp_info)
{
    void *pcbe = disp_info->cbios_ext;
    int  status = CBIOS_OK;
    int  bEDPLighted = 0, device_id = 0;
    CBIOS_BACKLIGHT_PARA  blPara = {0};
    unsigned int brightness = 0;
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device*  drm = zx_card->drm_dev;
    struct drm_connector* connector = NULL;
    zx_connector_t*  zx_connector = NULL;
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    struct drm_connector_list_iter conn_iter;
#endif

    mutex_lock(&drm->mode_config.mutex);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_begin(drm, &conn_iter);
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    drm_for_each_connector_iter(connector, &conn_iter)
#else

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    drm_for_each_connector(connector, drm)
#else
    list_for_each_entry(connector, &drm->mode_config.connector_list, head)
#endif    

#endif
    {
        zx_connector = to_zx_connector(connector);
        if(zx_connector->monitor_type == UT_OUTPUT_TYPE_PANEL)
        {
            bEDPLighted = 1;
            device_id = zx_connector->output_type;
            break;
        }
    }
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_end(&conn_iter);
#endif

    mutex_unlock(&drm->mode_config.mutex);

    if(bEDPLighted)
    {
        blPara.Size = sizeof(CBIOS_DSI_BACKLIGHT_PARA);
        blPara.DeviceId = device_id;

        status = CBiosGetBacklight(pcbe, &blPara);

        brightness = blPara.BacklightValue;
    }


    return brightness;
}

void  disp_cbios_query_brightness_caps(disp_info_t* disp_info, zx_brightness_caps_t *brightness_caps)
{
    void *pcbe = disp_info->cbios_ext;
    int  status = CBIOS_OK;
    int  bEDPLighted = 0, device_id = 0;
    CBiosMonitorAttribute  monitorAttr = {0};
    zx_card_t*  zx_card = disp_info->zx_card;
    struct drm_device*  drm = zx_card->drm_dev;
    struct drm_connector* connector = NULL;
    zx_connector_t*  zx_connector = NULL;
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    struct drm_connector_list_iter conn_iter;
#endif

    mutex_lock(&drm->mode_config.mutex);

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_begin(drm, &conn_iter);
#endif

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    drm_for_each_connector_iter(connector, &conn_iter)
#else

#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 8, 0)
    drm_for_each_connector(connector, drm)
#else
    list_for_each_entry(connector, &drm->mode_config.connector_list, head)
#endif    

#endif
    {
        zx_connector = to_zx_connector(connector);
        if(zx_connector->monitor_type == UT_OUTPUT_TYPE_PANEL)
        {
            bEDPLighted = 1;
            device_id = zx_connector->output_type;
            break;
        }
    }
#if DRM_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
    drm_connector_list_iter_end(&conn_iter);
#endif

    mutex_unlock(&drm->mode_config.mutex);

    if (bEDPLighted)
    {
        monitorAttr.Size = sizeof(monitorAttr);
        monitorAttr.ActiveDevId = device_id;

        status = CBiosQueryMonitorAttribute(pcbe, &monitorAttr);

        brightness_caps->support_brightness_ctrl = monitorAttr.bSupportBLCtrl;
        brightness_caps->max_brightness_value = monitorAttr.MaxBLLevel;
        brightness_caps->min_brightness_value = monitorAttr.MinBLLevel;
    }
}

int disp_wait_idle(void *_disp_info)
{
    disp_info_t *disp_info = _disp_info;
    
    unsigned long timeout_j = jiffies + msecs_to_jiffies(50) + 1;
    unsigned int  in_vblank;
    int ret = 0;
    zx_get_counter_t   get_cnt = {0};
    unsigned int i = 0;


    if(!disp_info)
    {
        zx_info("why disp_info is null, 0x%x  0x%x\n",disp_info,_disp_info);
        return -1;
    }

    i = 0;
    while(i < MAX_CORE_CRTCS)
    {
        if(disp_info->active_output[i])
        {
            break;
        }
        i++;
    }

    //case1): has one active crtc, wait the crtc's vblank
    //case2): no active crtc,  no need to wait.
    if(i != MAX_CORE_CRTCS)  //has active crtc
    {    
        get_cnt.crtc_index = i;
        get_cnt.in_vblk = &in_vblank;

        disp_cbios_get_counter(disp_info, &get_cnt);

        while(in_vblank == 1)
        {
            if(time_after(jiffies, timeout_j))
            {
                zx_error("wait in vblank tiemout \n");
                ret = -ETIMEDOUT;
                break;
            }
            disp_cbios_get_counter(disp_info, &get_cnt);
        }

        timeout_j = jiffies + msecs_to_jiffies(50) + 1;

        while(in_vblank == 0)
        {
            if(time_after(jiffies, timeout_j))
            {
                zx_error("wait in active timeout\n");
                ret = -ETIMEDOUT;
                break;
            }
            disp_cbios_get_counter(disp_info, &get_cnt);
        }   
    }

    return ret;
}

int disp_cbios_i2c_ctrl(disp_info_t *disp_info, zx_i2c_param_t *i2c_param)
{
    CBIOS_PARAM_I2C_DATA i2c_data = {0};
    int cb_status = CBIOS_OK;
    i2c_data.bUseDevType = i2c_param->use_dev_type;
    i2c_data.DeviceId = i2c_param->device_id;
    i2c_data.PortNumber = i2c_param->port;
    i2c_data.SlaveAddress = i2c_param->slave_addr;
    i2c_data.OffSet = i2c_param->offset;
    i2c_data.Buffer = i2c_param->buf;
    i2c_data.BufferLen = i2c_param->buf_len;
    i2c_data.bHDCPEnable = i2c_param->use_hdcp;
    switch (i2c_param->request_type)
    {
    case ZX_I2C_DDCCI:
        i2c_data.RequestType = CBIOS_I2CDDCCI;
        break;
    case ZX_I2C_HDCP:
        i2c_data.RequestType = CBIOS_I2CHDCP;
        break;
    default:
        i2c_data.RequestType = CBIOS_I2CNORMAL;
        break;
    }
    if (i2c_param->op == ZX_I2C_WRITE)
        cb_status =  CBiosI2CDataWrite(disp_info->cbios_ext, &i2c_data);
    else
        cb_status =  CBiosI2CDataRead(disp_info->cbios_ext, &i2c_data);
    return (cb_status == CBIOS_OK) ? S_OK : E_FAIL;
}

