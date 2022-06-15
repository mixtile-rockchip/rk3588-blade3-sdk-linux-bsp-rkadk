/*
 * Copyright (c) 2022 Rockchip, Inc. All Rights Reserved.
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

#include "rkadk_osd.h"
#include "rkadk_log.h"
#include "rkadk_media_comm.h"
#include "rkadk_param.h"
#include <string.h>

struct RKADK_FORMAT_MAP {
    RKADK_FORMAT_E Format;
    PIXEL_FORMAT_E enPixelFormat;
};

static const struct RKADK_FORMAT_MAP fmt[] = {
    {RKADK_FMT_ARGB1555, RK_FMT_ARGB1555},
    {RKADK_FMT_ABGR1555, RK_FMT_ABGR1555},
    {RKADK_FMT_RGBA5551, RK_FMT_RGBA5551},
    {RKADK_FMT_BGRA5551, RK_FMT_BGRA5551},
    {RKADK_FMT_ARGB4444, RK_FMT_ARGB4444},
    {RKADK_FMT_ABGR4444, RK_FMT_ABGR4444},
    {RKADK_FMT_RGBA4444, RK_FMT_RGBA4444},
    {RKADK_FMT_BGRA4444, RK_FMT_BGRA4444},
    {RKADK_FMT_ARGB8888, RK_FMT_ARGB8888},
    {RKADK_FMT_ABGR8888, RK_FMT_ABGR8888},
    {RKADK_FMT_RGBA8888, RK_FMT_RGBA8888},
    {RKADK_FMT_BGRA8888, RK_FMT_BGRA8888},
    {RKADK_FMT_2BPP, RK_FMT_2BPP}
};

static PIXEL_FORMAT_E TO_RK_FORMAT_FMT(RKADK_FORMAT_E Format) {
  int i;
	for (i = 0; i < sizeof(fmt) / sizeof(fmt[0]); i++) {
		if (fmt[i].Format == Format)
			return fmt[i].enPixelFormat;
	}
	return RK_FMT_BUTT;
}

RKADK_S32 RKADK_OSD_Init(RKADK_U32 u32OsdId, RKADK_OSD_ATTR_S *pstOsdAttr) {
  int ret;
  RGN_ATTR_S stRgnAttr;
  RGN_HANDLE RgnHandle = 0;

  memset(&stRgnAttr, 0, sizeof(RGN_ATTR_S));
  stRgnAttr.enType = OVERLAY_RGN;
  stRgnAttr.unAttr.stOverlay.enPixelFmt = TO_RK_FORMAT_FMT(pstOsdAttr->Format);
  stRgnAttr.unAttr.stOverlay.stSize.u32Width  = UPALIGNTO(pstOsdAttr->Width, 16);
  stRgnAttr.unAttr.stOverlay.stSize.u32Height = UPALIGNTO(pstOsdAttr->Height, 16);
  RgnHandle = u32OsdId;

  ret = RK_MPI_RGN_Create(RgnHandle, &stRgnAttr);
  if (RK_SUCCESS != ret) {
    RKADK_LOGE("RK_MPI_RGN_Create (%d) failed with %#x!", RgnHandle, ret);
    RK_MPI_RGN_Destroy(RgnHandle);
    return ret;
  }

  return 0;
}

RKADK_S32 RKADK_OSD_Deinit(RKADK_U32 u32OsdId) {
  int ret;
  RGN_HANDLE RgnHandle = 0;
  RgnHandle = u32OsdId;

  ret = RK_MPI_RGN_Destroy(RgnHandle);
  if (0 != ret) {
    RK_LOGE("RK_MPI_RGN_Destroy [%d] failed with %#x", RgnHandle, ret);
    return ret;
  }

  return 0;
}

RKADK_S32 RKADK_OSD_AttachToStream(RKADK_U32 u32OsdId, RKADK_U32 u32CamId,
      RKADK_STREAM_TYPE_E enStrmType, RKADK_OSD_STREAM_ATTR_S *pstOsdStreamAttr) {
  int ret;
  RGN_HANDLE RgnHandle = 0;
  RGN_CHN_ATTR_S stRgnChnAttr;
  MPP_CHN_S stMppChn;
  RgnHandle = u32OsdId;
  RKADK_PARAM_COMM_CFG_S *pstCommCfg = RKADK_PARAM_GetCommCfg();
  RKADK_PARAM_REC_CFG_S *pstRecCfg =
      RKADK_PARAM_GetRecCfg(u32CamId);
  RKADK_PARAM_PHOTO_CFG_S *pstPhotoCfg =
      RKADK_PARAM_GetPhotoCfg(u32CamId);
  RKADK_PARAM_STREAM_CFG_S *pstLiveCfg =
      RKADK_PARAM_GetStreamCfg(u32CamId, RKADK_STREAM_TYPE_LIVE);
  RKADK_PARAM_STREAM_CFG_S *pstStreamCfg = RKADK_PARAM_GetStreamCfg(
      u32CamId, RKADK_STREAM_TYPE_PREVIEW);

  memset(&stMppChn, 0, sizeof(MPP_CHN_S));
  stMppChn.enModId = RK_ID_VENC;
  stMppChn.s32DevId = 0;

  switch (enStrmType) {
    case RKADK_STREAM_TYPE_VIDEO_MAIN:
      stMppChn.s32ChnId = pstRecCfg->attribute[0].venc_chn;
      break;
    case RKADK_STREAM_TYPE_VIDEO_SUB:
      stMppChn.s32ChnId = pstRecCfg->attribute[1].venc_chn;
      break;
    case RKADK_STREAM_TYPE_SNAP:
      stMppChn.s32ChnId = pstPhotoCfg->venc_chn;
      break;
    case RKADK_STREAM_TYPE_PREVIEW:
      stMppChn.s32ChnId = pstStreamCfg->attribute.venc_chn;
      break;
    case RKADK_STREAM_TYPE_LIVE:
      stMppChn.s32ChnId = pstLiveCfg->attribute.venc_chn;
      break;
    default:
      RKADK_LOGE("unsuport other stream type");
      break;
  }

  memset(&stRgnChnAttr, 0, sizeof(RGN_CHN_ATTR_S));
  if (pstCommCfg->osd)
    stRgnChnAttr.bShow = RK_TRUE;
  else
    stRgnChnAttr.bShow = RK_FALSE;
  stRgnChnAttr.enType = OVERLAY_RGN;
  stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = pstOsdStreamAttr->Origin_X;
  stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = pstOsdStreamAttr->Origin_Y;

  ret = RK_MPI_RGN_AttachToChn(RgnHandle, &stMppChn, &stRgnChnAttr);
    if (RK_SUCCESS != ret) {
      RKADK_LOGE("RK_MPI_RGN_AttachToChn (%d) failed with %#x!", RgnHandle, ret);
      return ret;
  }

  return 0;
}

RKADK_S32 RKADK_OSD_DettachFromStream(RKADK_U32 u32OsdId, RKADK_U32 u32CamId,
      RKADK_STREAM_TYPE_E enStrmType) {
  int ret;
  RGN_HANDLE RgnHandle = 0;
  RGN_CHN_ATTR_S stRgnChnAttr;
  MPP_CHN_S stMppChn;
  RgnHandle = u32OsdId;
  RKADK_PARAM_COMM_CFG_S *pstCommCfg = RKADK_PARAM_GetCommCfg();
  RKADK_PARAM_REC_CFG_S *pstRecCfg =
      RKADK_PARAM_GetRecCfg(u32CamId);
  RKADK_PARAM_PHOTO_CFG_S *pstPhotoCfg =
      RKADK_PARAM_GetPhotoCfg(u32CamId);
  RKADK_PARAM_STREAM_CFG_S *pstLiveCfg =
      RKADK_PARAM_GetStreamCfg(u32CamId, RKADK_STREAM_TYPE_LIVE);
  RKADK_PARAM_STREAM_CFG_S *pstStreamCfg = RKADK_PARAM_GetStreamCfg(
      u32CamId, RKADK_STREAM_TYPE_PREVIEW);

  memset(&stMppChn, 0, sizeof(MPP_CHN_S));
  stMppChn.enModId = RK_ID_VENC;
  stMppChn.s32DevId = 0;

  switch (enStrmType) {
    case RKADK_STREAM_TYPE_VIDEO_MAIN:
      stMppChn.s32ChnId = pstRecCfg->attribute[0].venc_chn;
      break;
    case RKADK_STREAM_TYPE_VIDEO_SUB:
      stMppChn.s32ChnId = pstRecCfg->attribute[1].venc_chn;
      break;
    case RKADK_STREAM_TYPE_SNAP:
      stMppChn.s32ChnId = pstPhotoCfg->venc_chn;
      break;
    case RKADK_STREAM_TYPE_PREVIEW:
      stMppChn.s32ChnId = pstStreamCfg->attribute.venc_chn;
      break;
    case RKADK_STREAM_TYPE_LIVE:
      stMppChn.s32ChnId = pstLiveCfg->attribute.venc_chn;
      break;
    default:
      RKADK_LOGE("unsuport other stream type");
      break;
  }

  ret = RK_MPI_RGN_DetachFromChn(RgnHandle, &stMppChn);
    if (RK_SUCCESS != ret) {
      RKADK_LOGE("RK_MPI_RGN_AttachToChn (%d) failed with %#x!", RgnHandle, ret);
      return ret;
  }

  return 0;
}

RKADK_S32 RKADK_OSD_Update(RKADK_U32 u32OsdId, RKADK_OSD_ATTR_S *pstOsdAttr) {
  int ret;
  RGN_HANDLE RgnHandle = 0;
  RgnHandle = u32OsdId;
  BITMAP_S stBitmap;

  memset(&stBitmap, 0, sizeof(BITMAP_S));
  stBitmap.enPixelFormat = TO_RK_FORMAT_FMT(pstOsdAttr->Format);
  stBitmap.u32Width = pstOsdAttr->Width;
  stBitmap.u32Height = pstOsdAttr->Height;
  stBitmap.pData = pstOsdAttr->pData;

  ret = RK_MPI_RGN_SetBitMap(RgnHandle, &stBitmap);
  if (0 != ret) {
      RK_LOGE("RK_MPI_RGN_SetBitMap failed with %#x!", ret);
      return ret;
  }

  return 0;
}
