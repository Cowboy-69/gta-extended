/*
The MIT License (MIT)

Copyright (c) 2013-2015 Denilson das Mercês Amorim (aka LINK/2012)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#pragma once
#include "modloader.h"
#include <cstdint>

typedef struct {

    uint32_t size;

    int* p_gGameState;

    void (*PlayMovieInWindow)(int, const char*);
    int32_t (*CFileMgr_OpenFile)(const char* file, const char* mode);
    int (*CFileMgr_LoadFile)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    bool (*CTxdStore_LoadTxd)(int slot, const char* filename);
    void* (*CFileLoader_LoadAtomicFile2Return)(const char* filename);
    void (*InitialiseGame)();

    void* ms_aInfoForModel;
    uint32_t uMaxResources;
    DWORD* p_gCdStreamFlags;
    HANDLE* p_gCdStreamSema;
    void* p_gChannelRequestQ;
    void** p_gpReadInfo;
    BOOL* p_gbCdStreamOverlapped;
    BOOL* p_gbCdStreamAsync;
    void** p_ms_pStreamingBuffer;
    uint32_t* p_ms_streamingBufferSize;
    bool (*CDirectory__FindItem4)(void*, const char*, uint32_t*, uint32_t*);
    void (*CStreaming__LoadCdDirectory0)();
    void (*CStreaming__LoadCdDirectory2)(const char* filename, int cd_index);
    int32_t(*CdStreamRead)(int32_t channel, void* buffer, uint32_t offset, uint32_t size);

    void (*LoadCdDirectoryUsingCallbacks)(void* pUserData, int n, bool (*ReadEntry)(void*, void*, uint32_t),
        bool (*RegisterEntry)(void*, void*, bool), void (*RegisterSpecialEntry)(void*, void*));

    void (*CTimer__Resume)();
    void (*CTimer__Suspend)();
    void (*CStreaming__FlushChannels)();
    void (*CStreaming__RequestModel)(int32_t id, int32_t flags);
    void (*CStreaming__RemoveModel)(int32_t id);
    void (*CStreaming__LoadAllRequestedModels)(bool priority);
    void (*CStreaming__RemoveAllUnusedModels)();
    bool (*CStreaming__RemoveLeastUsedModel)();

} modloader_re3_addr_table_t;

typedef struct {

    uint32_t size;

    void (*PlayMovieInWindow_Logo)(int, const char*);
    void (*PlayMovieInWindow_GTAtitles)(int, const char*);
    int32_t (*OpenFile_GtaDat)(const char* file, const char* mode);
    int32_t (*OpenFile_DefaultDat)(const char* file, const char* mode);
    int32_t (*OpenFile_CarcolsDat)(const char* file, const char* mode);
    int32_t (*OpenFile_PedGrpDat)(const char* file, const char* mode);
    int32_t (*OpenFile_CullzoneDat)(const char* file, const char* mode);
    int32_t (*OpenFile_MainScm)(const char* file, const char* mode);
    int32_t (*OpenFile_WaterproDat)(const char* file, const char* mode);
    int32_t (*OpenFile_PedIfp)(const char* file, const char* mode);
    int (*LoadFile_FistfiteDat)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    int (*LoadFile_HandlingCfg)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    int (*LoadFile_PedDat)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    int (*LoadFile_ObjectDat)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    int (*LoadFile_PedStatsDat)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    int (*LoadFile_WeaponDat)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    int (*LoadFile_ParticleCfg)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    int (*LoadFile_SurfaceDat)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    int (*LoadFile_TimecycleDat)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    int (*LoadFile_FlightDat)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    int (*LoadFile_Flight2Dat)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    int (*LoadFile_Flight3Dat)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    int (*LoadFile_Flight4Dat)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    int (*LoadFile_TracksDat)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    int (*LoadFile_Tracks2Dat)(const char* file, uint8_t* buf, int maxlen, const char* mode);
    bool (*LoadTxd_FontsTxd)(int slot, const char* filename);
    bool (*LoadTxd_FontsPTxd)(int slot, const char* filename);
    bool (*LoadTxd_FontsRTxd)(int slot, const char* filename);
    bool (*LoadTxd_FontsJTxd)(int slot, const char* filename);
    bool (*LoadTxd_FrontendTxd)(int slot, const char* filename);
    bool (*LoadTxd_HudTxd)(int slot, const char* filename);
    bool (*LoadTxd_MenuTxd)(int slot, const char* filename);
    bool (*LoadTxd_ParticleTxd)(int slot, const char* filename);
    void* (*LoadAtomic2Return_ArrowDff)(const char* filename);
    void* (*LoadAtomic2Return_ZonecylbDff)(const char* filename);
    void (*InitialiseGame)();

    void (*CdStreamThread)();
    void (*LoadCdDirectory0)();
    void (*RegisterNextModelRead)(uint32_t);
    HANDLE(*AcquireNextModelFileHandle)();
    void (*FetchCdDirectory)(const char*, int);
    int32_t(*CdStreamRead)(int32_t channel, void* buffer, uint32_t offset, uint32_t size);

    void(*OnRequestSpecialModel)(uint32_t model_id, const char* model_name, uint32_t pos, uint32_t size);

    const char* (*GetCdDirectoryPath_Unsafe)(const char* filepath);
    const char* (*GetCdStreamPath_Unsafe)(const char* filepath);

    const char* (*RegisterAndGetAtomicFile_Unsafe)(const char* filepath);
    const char* (*RegisterAndGetClumpFile_Unsafe)(const char* filepath);
    const char* (*RegisterAndGetTexDiction_Unsafe)(const char* filepath);
    const char* (*RegisterAndGetColFile_Unsafe)(const char* filepath);
    const char* (*RegisterAndGetIdeFile_Unsafe)(const char* filepath);
    const char* (*RegisterAndGetIplFile_Unsafe)(const char* filepath);
    const char* (*RegisterAndGetGxtFile_Unsafe)(const char* filepath);
    const char* (*RegisterAndGetPlayerBmpFile_Unsafe)(const char* filepath);
    const char* (*RegisterAndGetSplashFile_Unsafe)(const char* filepath);

    int32_t (*CFileMgr__OpenFile0)(const char* file, const char* mode);
    int32_t (*CFileMgr__OpenFile1)(int* pThis, const char* file, const char* mode);

} modloader_re3_callback_table_t;

typedef struct modloader_re3_t {

    uint32_t size;
    uint32_t re3_version;

    modloader_t* modloader;
    modloader_re3_addr_table_t* re3_addr_table;
    modloader_re3_callback_table_t* callback_table;

    void (*Tick)(struct modloader_re3_t*);
    void (*Shutdown)(struct modloader_re3_t*);

    void* movies_plugin_ptr;

} modloader_re3_t;

typedef int (*modloader_fInitFromRE3)(modloader_re3_t*);