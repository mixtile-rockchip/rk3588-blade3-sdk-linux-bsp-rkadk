/*
 * Copyright (c) 2021 Rockchip, Inc. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef __RKADK_STORAGE_H__
#define __RKADK_STORAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "rkadk_common.h"

typedef struct {
  RKADK_CHAR cFolderPath[RKADK_MAX_FILE_PATH_LEN];
  RKADK_S32 s32Limit;
} RKADK_STR_FOLDER_ATTR;

typedef struct {
  RKADK_CHAR cMountPath[RKADK_MAX_FILE_PATH_LEN];
  RKADK_S32 s32FreeSizeDelMin;
  RKADK_S32 s32FreeSizeDelMax;
  RKADK_S32 s32AutoDel;
  RKADK_S32 s32FolderNum;
  RKADK_STR_FOLDER_ATTR *pstFolderAttr;
} RKADK_STR_DEV_ATTR;

typedef struct _RKADK_FILE_INFO {
  RKADK_CHAR filename[RKADK_MAX_FILE_PATH_LEN];
  off_t stSize;
} RKADK_FILE_INFO;

typedef struct {
  RKADK_CHAR path[RKADK_MAX_FILE_PATH_LEN];
  RKADK_S32 s32FileNum;
  RKADK_FILE_INFO *file;
} RKADK_FILE_LIST;

typedef enum {
  DISK_UNMOUNTED = 0,
  DISK_MOUNTED,
  DISK_MOUNT_BUTT,
} RKADK_ENUM_MOUNT;

RKADK_S32 RKADK_STORAGE_Init(RKADK_MW_PTR *ppHandle,
                             RKADK_STR_DEV_ATTR *pstDevAttr);

RKADK_S32 RKADK_STORAGE_Deinit(RKADK_MW_PTR pHandle);

RKADK_STR_DEV_ATTR RKADK_STORAGE_GetDevAttr(RKADK_MW_PTR pHandle);

RKADK_ENUM_MOUNT RKADK_STORAGE_GetMountStatus(RKADK_MW_PTR pHandle);

RKADK_S32 RKADK_STORAGE_GetSdcardSize(RKADK_MW_PTR *ppHandle,
                                      RKADK_S32 *totalSize,
                                      RKADK_S32 *freeSize);

RKADK_S32 RKADK_STORAGE_GetFileList(RKADK_FILE_LIST *list,
                                    RKADK_MW_PTR pHandle);

RKADK_S32 RKADK_STORAGE_FreeFileList(RKADK_FILE_LIST list);

#ifdef __cplusplus
}
#endif
#endif