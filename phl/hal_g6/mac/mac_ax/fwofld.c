/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 ******************************************************************************/

#include "fwdl.h"

static u32 get_io_ofld_cap(struct mac_ax_adapter *adapter, u32 *val)
{
#define MAC_AX_IO_OFLD_MAJ_VER 0
#define MAC_AX_IO_OFLD_MIN_VER 10
#define MAC_AX_IO_OFLD_SUB_VER 3
#define MAC_AX_IO_OFLD_SUB_IDX 0
	struct mac_ax_fw_info *fw_info = &adapter->fw_info;

	if (fw_info->minor_ver > MAC_AX_IO_OFLD_MIN_VER) {
		*val |= FW_CAP_IO_OFLD;
		return MACSUCCESS;
	}

	if (fw_info->minor_ver == MAC_AX_IO_OFLD_MIN_VER &&
	    fw_info->sub_ver >= MAC_AX_IO_OFLD_SUB_VER)
		*val |= FW_CAP_IO_OFLD;

	return MACSUCCESS;
}

u32 mac_get_fw_cap(struct mac_ax_adapter *adapter, u32 *val)
{
	*val = 0;
	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY)
		return MACFWNONRDY;

	get_io_ofld_cap(adapter, val);

	return MACSUCCESS;
}

u32 mac_reset_fwofld_state(struct mac_ax_adapter *adapter, u8 op)
{
	switch (op) {
	case FW_OFLD_OP_DUMP_EFUSE:
		adapter->sm.efuse_ofld = MAC_AX_OFLD_H2C_IDLE;
		break;

	case FW_OFLD_OP_PACKET_OFLD:
		adapter->sm.pkt_ofld = MAC_AX_OFLD_H2C_IDLE;
		break;

	case FW_OFLD_OP_READ_OFLD:
		adapter->sm.read_request = MAC_AX_OFLD_REQ_IDLE;
		adapter->sm.read_h2c = MAC_AX_OFLD_H2C_IDLE;
		break;

	case FW_OFLD_OP_WRITE_OFLD:
		adapter->sm.write_request = MAC_AX_OFLD_REQ_IDLE;
		adapter->sm.write_h2c = MAC_AX_OFLD_H2C_IDLE;
		break;

	case FW_OFLD_OP_CONF_OFLD:
		adapter->sm.conf_request = MAC_AX_OFLD_REQ_IDLE;
		adapter->sm.conf_h2c = MAC_AX_OFLD_H2C_IDLE;
		break;

	default:
		return MACNOITEM;
	}

	return MACSUCCESS;
}

u32 mac_check_fwofld_done(struct mac_ax_adapter *adapter, u8 op)
{
	struct mac_ax_pkt_ofld_info *ofld_info = &adapter->pkt_ofld_info;

	switch (op) {
	case FW_OFLD_OP_DUMP_EFUSE:
		if (adapter->sm.efuse_ofld == MAC_AX_OFLD_H2C_IDLE)
			return MACSUCCESS;
		break;

	case FW_OFLD_OP_PACKET_OFLD:
		if (ofld_info->last_op == PKT_OFLD_OP_READ) {
			if (adapter->sm.pkt_ofld == MAC_AX_OFLD_H2C_DONE)
				return MACSUCCESS;
		} else {
			if (adapter->sm.pkt_ofld == MAC_AX_OFLD_H2C_IDLE)
				return MACSUCCESS;
		}
		break;
	case FW_OFLD_OP_READ_OFLD:
		if (adapter->sm.read_h2c == MAC_AX_OFLD_H2C_DONE)
			return MACSUCCESS;
		break;
	case FW_OFLD_OP_WRITE_OFLD:
		if (adapter->sm.write_h2c == MAC_AX_OFLD_H2C_IDLE)
			return MACSUCCESS;
		break;
	case FW_OFLD_OP_CONF_OFLD:
		if (adapter->sm.conf_h2c == MAC_AX_OFLD_H2C_IDLE)
			return MACSUCCESS;
		break;
	default:
		return MACNOITEM;
	}

	return MACPROCBUSY;
}

static u32 cnv_write_ofld_state(struct mac_ax_adapter *adapter, u8 dest)
{
	u8 state;

	state = adapter->sm.write_request;

	if (state > MAC_AX_OFLD_REQ_CLEANED)
		return MACPROCERR;

	if (dest == MAC_AX_OFLD_REQ_IDLE) {
		if (state != MAC_AX_OFLD_REQ_H2C_SENT)
			return MACPROCERR;
	} else if (dest == MAC_AX_OFLD_REQ_CLEANED) {
		if (state == MAC_AX_OFLD_REQ_H2C_SENT)
			return MACPROCERR;
	} else if (dest == MAC_AX_OFLD_REQ_CREATED) {
		if (state == MAC_AX_OFLD_REQ_IDLE ||
		    state == MAC_AX_OFLD_REQ_H2C_SENT)
			return MACPROCERR;
	} else if (dest == MAC_AX_OFLD_REQ_H2C_SENT) {
		if (state != MAC_AX_OFLD_REQ_CREATED)
			return MACPROCERR;
	}

	adapter->sm.write_request = dest;

	return MACSUCCESS;
}

u32 mac_clear_write_request(struct mac_ax_adapter *adapter)
{
	if (adapter->sm.write_request == MAC_AX_OFLD_REQ_H2C_SENT)
		return MACPROCERR;

	if (cnv_write_ofld_state(adapter, MAC_AX_OFLD_REQ_CLEANED)
	    != MACSUCCESS)
		return MACPROCERR;

	PLTFM_FREE(adapter->write_ofld_info.buf,
		   adapter->write_ofld_info.buf_size);
	adapter->write_ofld_info.buf = NULL;
	adapter->write_ofld_info.buf_wptr = NULL;
	adapter->write_ofld_info.last_req = NULL;
	adapter->write_ofld_info.buf_size = 0;
	adapter->write_ofld_info.avl_buf_size = 0;
	adapter->write_ofld_info.used_size = 0;
	adapter->write_ofld_info.req_num = 0;

	return MACSUCCESS;
}

u32 mac_add_write_request(struct mac_ax_adapter *adapter,
			  struct mac_ax_write_req *req, u8 *value, u8 *mask)
{
	struct mac_ax_write_ofld_info *ofld_info = &adapter->write_ofld_info;
	struct fwcmd_write_ofld_req *write_ptr;
	u32 data_len = 0;
	u8 state;

	state = adapter->sm.write_request;

	if (!(state == MAC_AX_OFLD_REQ_CREATED ||
	      state == MAC_AX_OFLD_REQ_CLEANED)) {
		return MACPROCERR;
	}

	if (!ofld_info->buf) {
		ofld_info->buf = (u8 *)PLTFM_MALLOC(WRITE_OFLD_MAX_LEN);
		if (!ofld_info->buf)
			return MACNPTR;
		ofld_info->buf_wptr = ofld_info->buf;
		ofld_info->buf_size = WRITE_OFLD_MAX_LEN;
		ofld_info->avl_buf_size = WRITE_OFLD_MAX_LEN;
		ofld_info->used_size = 0;
		ofld_info->req_num = 0;
	}

	data_len = sizeof(struct mac_ax_write_req);
	data_len += req->value_len;
	if (req->mask_en == 1)
		data_len += req->value_len;

	if (ofld_info->avl_buf_size < data_len)
		return MACNOBUF;

	if (!value)
		return MACNPTR;

	if (req->mask_en == 1 && !mask)
		return MACNPTR;

	if (cnv_write_ofld_state(adapter,
				 MAC_AX_OFLD_REQ_CREATED) != MACSUCCESS)
		return MACPROCERR;

	if (ofld_info->req_num != 0)
		ofld_info->last_req->ls = 0;

	ofld_info->last_req = (struct mac_ax_write_req *)ofld_info->buf_wptr;

	req->ls = 1;

	write_ptr = (struct fwcmd_write_ofld_req *)ofld_info->buf_wptr;
	write_ptr->dword0 =
	cpu_to_le32(SET_WORD(req->value_len,
			     FWCMD_H2C_WRITE_OFLD_REQ_VALUE_LEN) |
		    SET_WORD(req->ofld_id,
			     FWCMD_H2C_WRITE_OFLD_REQ_OFLD_ID) |
		    SET_WORD(req->entry_num,
			     FWCMD_H2C_WRITE_OFLD_REQ_ENTRY_NUM) |
		    req->polling | req->mask_en | req->ls
	);

	write_ptr->dword1 =
	cpu_to_le32(SET_WORD(req->offset,
			     FWCMD_H2C_WRITE_OFLD_REQ_OFFSET)
	);

	ofld_info->buf_wptr += sizeof(struct mac_ax_write_req);
	ofld_info->avl_buf_size -= sizeof(struct mac_ax_write_req);
	ofld_info->used_size += sizeof(struct mac_ax_write_req);

	PLTFM_MEMCPY(ofld_info->buf_wptr, value, req->value_len);

	ofld_info->buf_wptr += req->value_len;
	ofld_info->avl_buf_size -= req->value_len;
	ofld_info->used_size += req->value_len;

	if (req->mask_en == 1) {
		PLTFM_MEMCPY(ofld_info->buf_wptr, mask, req->value_len);
		ofld_info->buf_wptr += req->value_len;
		ofld_info->avl_buf_size -= req->value_len;
		ofld_info->used_size += req->value_len;
	}

	ofld_info->req_num++;

	return MACSUCCESS;
}

u32 mac_write_ofld(struct mac_ax_adapter *adapter)
{
	u8 *buf;
	u32 ret;
	#if MAC_AX_PHL_H2C
	struct rtw_h2c_pkt *h2cb;
	#else
	struct h2c_buf *h2cb;
	#endif
	struct mac_ax_write_ofld_info *ofld_info = &adapter->write_ofld_info;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY)
		return MACNOFW;

	if (ofld_info->used_size + FWCMD_HDR_LEN > READ_OFLD_MAX_LEN)
		return MACBUFSZ;

	if (adapter->sm.write_h2c != MAC_AX_OFLD_H2C_IDLE)
		return MACPROCERR;

	if (adapter->sm.write_request != MAC_AX_OFLD_REQ_CREATED)
		return MACPROCERR;

	if (cnv_write_ofld_state(adapter,
				 MAC_AX_OFLD_REQ_H2C_SENT) != MACSUCCESS)
		return MACPROCERR;

	adapter->sm.write_h2c = MAC_AX_OFLD_H2C_SENDING;

	h2cb = h2cb_alloc(adapter, H2CB_CLASS_LONG_DATA);
	if (!h2cb)
		return MACNPTR;

	buf = h2cb_put(h2cb, ofld_info->used_size);
	if (!buf) {
		ret = MACNOBUF;
		goto fail;
	}

	PLTFM_MEMCPY(buf, ofld_info->buf, ofld_info->used_size);

	ret = h2c_pkt_set_hdr(adapter, h2cb,
			      FWCMD_TYPE_H2C, FWCMD_H2C_CAT_MAC,
			      FWCMD_H2C_CL_FW_OFLD, FWCMD_H2C_FUNC_WRITE_OFLD,
			      1, 1);

	if (ret)
		goto fail;

	// return MACSUCCESS if h2c aggregation is enabled and enqueued successfully.
	// H2C shall be sent by mac_h2c_agg_tx.
	ret = h2c_agg_enqueue(adapter, h2cb);
	if (ret == MACSUCCESS)
		return MACSUCCESS;

	ret = h2c_pkt_build_txd(adapter, h2cb);
	if (ret)
		goto fail;

	#if MAC_AX_PHL_H2C
	ret = PLTFM_TX(h2cb);
	#else
	ret = PLTFM_TX(h2cb->data, h2cb->len);
	#endif
	if (ret) {
		PLTFM_MSG_ERR("[ERR]platform tx: %d\n", ret);
		adapter->sm.write_request = MAC_AX_OFLD_REQ_IDLE;
		adapter->sm.write_h2c = MAC_AX_OFLD_H2C_IDLE;
		goto fail;
	}

	h2cb_free(adapter, h2cb);

	if (cnv_write_ofld_state(adapter, MAC_AX_OFLD_REQ_IDLE) != MACSUCCESS)
		return MACPROCERR;

	h2c_end_flow(adapter);

	return MACSUCCESS;
fail:
	h2cb_free(adapter, h2cb);

	return ret;
}

static u32 cnv_conf_ofld_state(struct mac_ax_adapter *adapter, u8 dest)
{
	u8 state;

	state = adapter->sm.conf_request;

	if (state > MAC_AX_OFLD_REQ_CLEANED)
		return MACPROCERR;

	if (dest == MAC_AX_OFLD_REQ_IDLE) {
		if (state != MAC_AX_OFLD_REQ_H2C_SENT)
			return MACPROCERR;
	} else if (dest == MAC_AX_OFLD_REQ_CLEANED) {
		if (state == MAC_AX_OFLD_REQ_H2C_SENT)
			return MACPROCERR;
	} else if (dest == MAC_AX_OFLD_REQ_CREATED) {
		if (state == MAC_AX_OFLD_REQ_IDLE ||
		    state == MAC_AX_OFLD_REQ_H2C_SENT)
			return MACPROCERR;
	} else if (dest == MAC_AX_OFLD_REQ_H2C_SENT) {
		if (state != MAC_AX_OFLD_REQ_CREATED)
			return MACPROCERR;
	}

	adapter->sm.conf_request = dest;

	return MACSUCCESS;
}

u32 mac_clear_conf_request(struct mac_ax_adapter *adapter)
{
	if (adapter->sm.conf_request == MAC_AX_OFLD_REQ_H2C_SENT)
		return MACPROCERR;

	if (cnv_conf_ofld_state(adapter, MAC_AX_OFLD_REQ_CLEANED) !=
	    MACSUCCESS)
		return MACPROCERR;

	PLTFM_FREE(adapter->conf_ofld_info.buf,
		   adapter->conf_ofld_info.buf_size);
	adapter->conf_ofld_info.buf = NULL;
	adapter->conf_ofld_info.buf_wptr = NULL;
	adapter->conf_ofld_info.buf_size = 0;
	adapter->conf_ofld_info.avl_buf_size = 0;
	adapter->conf_ofld_info.used_size = 0;
	adapter->conf_ofld_info.req_num = 0;

	return MACSUCCESS;
}

u32 mac_add_conf_request(struct mac_ax_adapter *adapter,
			 struct mac_ax_conf_ofld_req *req)
{
	struct mac_ax_conf_ofld_info *ofld_info = &adapter->conf_ofld_info;
	struct fwcmd_conf_ofld_req_cmd *write_ptr;
	u8 state;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY)
		return MACNOFW;

	state = adapter->sm.conf_request;

	if (!(state == MAC_AX_OFLD_REQ_CREATED ||
	      state == MAC_AX_OFLD_REQ_CLEANED)) {
		return MACPROCERR;
	}

	if (!ofld_info->buf) {
		ofld_info->buf = (u8 *)PLTFM_MALLOC(CONF_OFLD_MAX_LEN);
		if (!ofld_info->buf)
			return MACNPTR;
		ofld_info->buf_wptr = ofld_info->buf;
		ofld_info->buf_size = CONF_OFLD_MAX_LEN;
		ofld_info->avl_buf_size = CONF_OFLD_MAX_LEN;
		ofld_info->used_size = 0;
		ofld_info->req_num = 0;
	}

	if (ofld_info->avl_buf_size < sizeof(struct mac_ax_conf_ofld_req))
		return MACNOBUF;

	if (cnv_conf_ofld_state(adapter, MAC_AX_OFLD_REQ_CREATED) != MACSUCCESS)
		return MACPROCERR;

	write_ptr = (struct fwcmd_conf_ofld_req_cmd *)ofld_info->buf_wptr;
	write_ptr->dword0 =
	cpu_to_le32(SET_WORD(req->device,
			     FWCMD_H2C_CONF_OFLD_REQ_CMD_DEVICE)
	);

	write_ptr->dword1 =
	cpu_to_le32(SET_WORD(req->req.hioe.hioe_op,
			     FWCMD_H2C_CONF_OFLD_REQ_CMD_HIOE_OP) |
		    SET_WORD(req->req.hioe.inst_type,
			     FWCMD_H2C_CONF_OFLD_REQ_CMD_INST_TYPE) |
		    SET_WORD(req->req.hioe.data_mode,
			     FWCMD_H2C_CONF_OFLD_REQ_CMD_DATA_MODE)
	);

	write_ptr->dword2 = cpu_to_le32(req->req.hioe.param0.register_addr);

	write_ptr->dword3 =
	cpu_to_le32(SET_WORD(req->req.hioe.param1.byte_data_h,
			     FWCMD_H2C_CONF_OFLD_REQ_CMD_BYTE_DATA_H) |
		    SET_WORD(req->req.hioe.param2.byte_data_l,
			     FWCMD_H2C_CONF_OFLD_REQ_CMD_BYTE_DATA_L)
	);

	ofld_info->buf_wptr += sizeof(struct mac_ax_conf_ofld_req);
	ofld_info->avl_buf_size -= sizeof(struct mac_ax_conf_ofld_req);
	ofld_info->used_size += sizeof(struct mac_ax_conf_ofld_req);

	ofld_info->req_num++;

	return MACSUCCESS;
}

u32 mac_conf_ofld(struct mac_ax_adapter *adapter)
{
	u8 *buf;
	u32 ret;
	struct fwcmd_conf_ofld *write_ptr;
	#if MAC_AX_PHL_H2C
	struct rtw_h2c_pkt *h2cb;
	#else
	struct h2c_buf *h2cb;
	#endif
	struct mac_ax_conf_ofld_info *ofld_info = &adapter->conf_ofld_info;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY)
		return MACNOFW;

	if (ofld_info->used_size + FWCMD_HDR_LEN > CONF_OFLD_MAX_LEN)
		return MACBUFSZ;

	if (adapter->sm.conf_h2c != MAC_AX_OFLD_H2C_IDLE)
		return MACPROCERR;

	if (adapter->sm.conf_request != MAC_AX_OFLD_REQ_CREATED)
		return MACPROCERR;

	if (cnv_conf_ofld_state(adapter,
				MAC_AX_OFLD_REQ_H2C_SENT) != MACSUCCESS)
		return MACPROCERR;

	adapter->sm.conf_h2c = MAC_AX_OFLD_H2C_SENDING;

	h2cb = h2cb_alloc(adapter, H2CB_CLASS_LONG_DATA);
	if (!h2cb)
		return MACNPTR;

	buf = h2cb_put(h2cb, sizeof(struct mac_ax_conf_ofld_hdr));
	if (!buf) {
		ret = MACNOBUF;
		goto fail;
	}

	write_ptr = (struct fwcmd_conf_ofld *)buf;

	write_ptr->dword0 =
	cpu_to_le32(SET_WORD(ofld_info->req_num,
			     FWCMD_H2C_CONF_OFLD_PATTERN_COUNT));

	buf = h2cb_put(h2cb, ofld_info->used_size);
	if (!buf) {
		ret = MACNOBUF;
		goto fail;
	}

	PLTFM_MEMCPY(buf, ofld_info->buf, ofld_info->used_size);

	ret = h2c_pkt_set_hdr(adapter, h2cb,
			      FWCMD_TYPE_H2C, FWCMD_H2C_CAT_MAC,
			      FWCMD_H2C_CL_FW_OFLD, FWCMD_H2C_FUNC_CONF_OFLD,
			      1, 1);
	if (ret)
		goto fail;

	ret = h2c_pkt_build_txd(adapter, h2cb);
	if (ret)
		goto fail;

	#if MAC_AX_PHL_H2C
	ret = PLTFM_TX(h2cb);
	#else
	ret = PLTFM_TX(h2cb->data, h2cb->len);
	#endif
	if (ret) {
		PLTFM_MSG_ERR("[ERR]platform tx: %d\n", ret);
		adapter->sm.conf_request = MAC_AX_OFLD_REQ_IDLE;
		adapter->sm.conf_h2c = MAC_AX_OFLD_H2C_IDLE;
		goto fail;
	}

	h2cb_free(adapter, h2cb);

	if (cnv_conf_ofld_state(adapter, MAC_AX_OFLD_REQ_IDLE) != MACSUCCESS)
		return MACPROCERR;

	h2c_end_flow(adapter);

	return MACSUCCESS;
fail:
	h2cb_free(adapter, h2cb);

	return ret;
}

static inline void mac_pkt_ofld_set_bitmap(u8 *bitmap, u16 index)
{
	bitmap[index >> 3] |= (1 << (index & 7));
}

static inline void mac_pkt_ofld_unset_bitmap(u8 *bitmap, u16 index)
{
	bitmap[index >> 3] &= ~(1 << (index & 7));
}

static inline u8 mac_pkt_ofld_get_bitmap(u8 *bitmap, u16 index)
{
	return bitmap[index / 8] & (1 << (index & 7)) ? 1 : 0;
}

u32 mac_read_pkt_ofld(struct mac_ax_adapter *adapter, u8 id)
{
	u8 *buf;
	u32 ret;
	#if MAC_AX_PHL_H2C
	struct rtw_h2c_pkt *h2cb;
	#else
	struct h2c_buf *h2cb;
	#endif
	struct fwcmd_packet_ofld *write_ptr;
	struct mac_ax_pkt_ofld_info *ofld_info = &adapter->pkt_ofld_info;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY)
		return MACNOFW;

	if (id == PKT_OFLD_MAX_COUNT - 1)
		return MACNOITEM;

	if (mac_pkt_ofld_get_bitmap(ofld_info->id_bitmap, id) == 0)
		return MACNOITEM;

	if (adapter->sm.pkt_ofld != MAC_AX_OFLD_H2C_IDLE)
		return MACPROCERR;

	adapter->sm.pkt_ofld = MAC_AX_OFLD_H2C_SENDING;

	h2cb = h2cb_alloc(adapter, H2CB_CLASS_CMD);
	if (!h2cb)
		return MACNPTR;

	buf = h2cb_put(h2cb, sizeof(struct mac_ax_pkt_ofld_hdr));
	if (!buf) {
		ret = MACNOBUF;
		goto fail;
	}

	write_ptr = (struct fwcmd_packet_ofld *)buf;
	write_ptr->dword0 =
	cpu_to_le32(SET_WORD(id, FWCMD_H2C_PACKET_OFLD_PKT_IDX) |
		    SET_WORD(PKT_OFLD_OP_READ, FWCMD_H2C_PACKET_OFLD_PKT_OP)
	);

	ret = h2c_pkt_set_hdr(adapter, h2cb,
			      FWCMD_TYPE_H2C, FWCMD_H2C_CAT_MAC,
			      FWCMD_H2C_CL_FW_OFLD,
			      FWCMD_H2C_FUNC_PACKET_OFLD,
			      1, 1);
	if (ret)
		goto fail;

	ret = h2c_pkt_build_txd(adapter, h2cb);
	if (ret)
		goto fail;

	#if MAC_AX_PHL_H2C
	ret = PLTFM_TX(h2cb);
	#else
	ret = PLTFM_TX(h2cb->data, h2cb->len);
	#endif
	if (ret) {
		PLTFM_MSG_ERR("[ERR]platform tx: %d\n", ret);
		adapter->sm.pkt_ofld = MAC_AX_OFLD_H2C_IDLE;
		goto fail;
	}

	h2cb_free(adapter, h2cb);

	ofld_info->last_op = PKT_OFLD_OP_READ;

	h2c_end_flow(adapter);

	return MACSUCCESS;
fail:
	h2cb_free(adapter, h2cb);

	return ret;
}

u32 mac_del_pkt_ofld(struct mac_ax_adapter *adapter, u8 id)
{
	u8 *buf;
	u32 ret;
	#if MAC_AX_PHL_H2C
	struct rtw_h2c_pkt *h2cb;
	#else
	struct h2c_buf *h2cb;
	#endif
	struct fwcmd_packet_ofld *write_ptr;
	struct mac_ax_pkt_ofld_info *ofld_info = &adapter->pkt_ofld_info;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY)
		return MACNOFW;

	if (id == PKT_OFLD_MAX_COUNT - 1)
		return MACNOITEM;

	if (mac_pkt_ofld_get_bitmap(ofld_info->id_bitmap, id) == 0)
		return MACNOITEM;

	if (ofld_info->used_id_count == 0)
		return MACNOITEM;

	if (adapter->sm.pkt_ofld != MAC_AX_OFLD_H2C_IDLE)
		return MACPROCERR;

	adapter->sm.pkt_ofld = MAC_AX_OFLD_H2C_SENDING;

	h2cb = h2cb_alloc(adapter, H2CB_CLASS_CMD);
	if (!h2cb)
		return MACNPTR;

	buf = h2cb_put(h2cb, sizeof(struct mac_ax_pkt_ofld_hdr));
	if (!buf) {
		ret = MACNOBUF;
		goto fail;
	}

	write_ptr = (struct fwcmd_packet_ofld *)buf;
	write_ptr->dword0 =
	cpu_to_le32(SET_WORD(id, FWCMD_H2C_PACKET_OFLD_PKT_IDX) |
		    SET_WORD(PKT_OFLD_OP_DEL, FWCMD_H2C_PACKET_OFLD_PKT_OP)
	);

	ret = h2c_pkt_set_hdr(adapter, h2cb,
			      FWCMD_TYPE_H2C, FWCMD_H2C_CAT_MAC,
			      FWCMD_H2C_CL_FW_OFLD, FWCMD_H2C_FUNC_PACKET_OFLD,
			      1, 1);
	if (ret)
		goto fail;

	ret = h2c_pkt_build_txd(adapter, h2cb);
	if (ret)
		goto fail;

	#if MAC_AX_PHL_H2C
	ret = PLTFM_TX(h2cb);
	#else
	ret = PLTFM_TX(h2cb->data, h2cb->len);
	#endif
	if (ret) {
		PLTFM_MSG_ERR("[ERR]platform tx: %d\n", ret);
		adapter->sm.pkt_ofld = MAC_AX_OFLD_H2C_IDLE;
		goto fail;
	}

	h2cb_free(adapter, h2cb);

	ofld_info->last_op = PKT_OFLD_OP_DEL;

	h2c_end_flow(adapter);

	return MACSUCCESS;
fail:
	h2cb_free(adapter, h2cb);

	return ret;
}

u32 mac_add_pkt_ofld(struct mac_ax_adapter *adapter, u8 *pkt, u16 len, u8 *id)
{
	u8 *buf;
	u16 alloc_id;
	u32 ret;
	#if MAC_AX_PHL_H2C
	struct rtw_h2c_pkt *h2cb;
	#else
	struct h2c_buf *h2cb;
	#endif
	struct fwcmd_packet_ofld *write_ptr;
	struct mac_ax_pkt_ofld_info *ofld_info = &adapter->pkt_ofld_info;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY)
		return MACNOFW;

	if (ofld_info->free_id_count == 0)
		return MACNOBUF;

	if (adapter->sm.pkt_ofld != MAC_AX_OFLD_H2C_IDLE)
		return MACPROCERR;

	adapter->sm.pkt_ofld = MAC_AX_OFLD_H2C_SENDING;

	for (alloc_id = 0; alloc_id < PKT_OFLD_MAX_COUNT - 1; alloc_id++) {
		if (mac_pkt_ofld_get_bitmap(ofld_info->id_bitmap,
					    alloc_id) == 0)
			break;
	}

	PLTFM_MSG_TRACE("pkt ofld add. alloc_id: %d, free cnt: %d, use cnt: %d\n",
			alloc_id, ofld_info->free_id_count,
			ofld_info->used_id_count);

	h2cb = h2cb_alloc(adapter, H2CB_CLASS_DATA);
	if (!h2cb)
		return MACNPTR;

	buf = h2cb_put(h2cb, sizeof(struct mac_ax_pkt_ofld_hdr));
	if (!buf) {
		ret = MACNOBUF;
		goto fail;
	}

	write_ptr = (struct fwcmd_packet_ofld *)buf;
	write_ptr->dword0 =
	cpu_to_le32(SET_WORD((u8)alloc_id, FWCMD_H2C_PACKET_OFLD_PKT_IDX) |
		    SET_WORD(PKT_OFLD_OP_ADD, FWCMD_H2C_PACKET_OFLD_PKT_OP) |
		    SET_WORD(len, FWCMD_H2C_PACKET_OFLD_PKT_LENGTH)
	);

	*id = (u8)alloc_id;

	buf = h2cb_put(h2cb, len);
	if (!buf) {
		ret = MACNOBUF;
		goto fail;
	}

	PLTFM_MEMCPY(buf, pkt, len);

	ret = h2c_pkt_set_hdr(adapter, h2cb,
			      FWCMD_TYPE_H2C, FWCMD_H2C_CAT_MAC,
			      FWCMD_H2C_CL_FW_OFLD, FWCMD_H2C_FUNC_PACKET_OFLD,
			      1, 1);
	if (ret)
		goto fail;

	ret = h2c_pkt_build_txd(adapter, h2cb);
	if (ret)
		goto fail;

	#if MAC_AX_PHL_H2C
	ret = PLTFM_TX(h2cb);
	#else
	ret = PLTFM_TX(h2cb->data, h2cb->len);
	#endif
	if (ret) {
		PLTFM_MSG_ERR("[ERR]platform tx: %d\n", ret);
		adapter->sm.pkt_ofld = MAC_AX_OFLD_H2C_IDLE;
		goto fail;
	}

	h2cb_free(adapter, h2cb);

	ofld_info->last_op = PKT_OFLD_OP_ADD;

	return MACSUCCESS;
fail:
	h2cb_free(adapter, h2cb);

	return ret;
}

u32 mac_pkt_ofld_packet(struct mac_ax_adapter *adapter,
			u8 **pkt_buf, u16 *pkt_len, u8 *pkt_id)
{
	struct mac_ax_pkt_ofld_pkt *pkt_info = &adapter->pkt_ofld_pkt;
	*pkt_buf = NULL;

	if (adapter->sm.pkt_ofld != MAC_AX_OFLD_H2C_DONE)
		return MACPROCERR;

	*pkt_buf = (u8 *)PLTFM_MALLOC(pkt_info->pkt_len);
	if (!*pkt_buf)
		return MACBUFALLOC;

	PLTFM_MEMCPY(*pkt_buf, pkt_info->pkt, pkt_info->pkt_len);

	*pkt_len = pkt_info->pkt_len;
	*pkt_id = pkt_info->pkt_id;

	adapter->sm.pkt_ofld = MAC_AX_OFLD_H2C_IDLE;

	return MACSUCCESS;
}

u32 mac_dump_efuse_ofld(struct mac_ax_adapter *adapter, u32 efuse_size,
			bool is_hidden)
{
	u32 ret, size;
	#if MAC_AX_PHL_H2C
	struct rtw_h2c_pkt *h2cb;
	#else
	struct h2c_buf *h2cb;
	#endif
	struct mac_ax_efuse_ofld_info *ofld_info = &adapter->efuse_ofld_info;
	u8 *buf;
	struct fwcmd_dump_efuse *write_ptr;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY)
		return MACNOFW;

	if (adapter->sm.efuse_ofld != MAC_AX_OFLD_H2C_IDLE)
		return MACPROCERR;

	adapter->sm.efuse_ofld = MAC_AX_OFLD_H2C_SENDING;

	size = efuse_size;
	if (!ofld_info->buf) {
		ofld_info->buf = (u8 *)PLTFM_MALLOC(size);
		if (!ofld_info->buf)
			return MACBUFALLOC;
	}

	h2cb = h2cb_alloc(adapter, H2CB_CLASS_CMD);
	if (!h2cb)
		return MACNPTR;

	buf = h2cb_put(h2cb, sizeof(struct mac_ax_pkt_ofld_hdr));
	if (!buf) {
		ret = MACNOBUF;
		goto fail;
	}

	write_ptr = (struct fwcmd_dump_efuse *)buf;
	write_ptr->dword0 =
	cpu_to_le32(SET_WORD(efuse_size, FWCMD_H2C_DUMP_EFUSE_DUMP_SIZE) |
		    (is_hidden ? FWCMD_H2C_DUMP_EFUSE_IS_HIDDEN : 0));

	ret = h2c_pkt_set_hdr(adapter, h2cb,
			      FWCMD_TYPE_H2C, FWCMD_H2C_CAT_MAC,
			      FWCMD_H2C_CL_FW_OFLD, FWCMD_H2C_FUNC_DUMP_EFUSE,
			      0, 0);
	if (ret)
		goto fail;

	ret = h2c_pkt_build_txd(adapter, h2cb);
	if (ret)
		goto fail;

	#if MAC_AX_PHL_H2C
	ret = PLTFM_TX(h2cb);
	#else
	ret = PLTFM_TX(h2cb->data, h2cb->len);
	#endif
	if (ret) {
		PLTFM_MSG_ERR("[ERR]platform tx\n");
		goto fail;
	}

	h2cb_free(adapter, h2cb);
	return MACSUCCESS;
fail:
	h2cb_free(adapter, h2cb);
	return ret;
}

u32 mac_efuse_ofld_map(struct mac_ax_adapter *adapter, u8 *efuse_map,
		       u32 efuse_size)
{
	u32 size = efuse_size;
	struct mac_ax_efuse_ofld_info *ofld_info = &adapter->efuse_ofld_info;

	if (adapter->sm.efuse_ofld != MAC_AX_OFLD_H2C_DONE)
		return MACPROCERR;

	PLTFM_MEMCPY(efuse_map, ofld_info->buf, size);

	adapter->sm.efuse_ofld = MAC_AX_OFLD_H2C_IDLE;

	return MACSUCCESS;
}

static u32 cnv_read_ofld_state(struct mac_ax_adapter *adapter, u8 dest)
{
	u8 state;

	state = adapter->sm.read_request;

	if (state > MAC_AX_OFLD_REQ_CLEANED)
		return MACPROCERR;

	if (dest == MAC_AX_OFLD_REQ_IDLE) {
		if (state != MAC_AX_OFLD_REQ_H2C_SENT)
			return MACPROCERR;
	} else if (dest == MAC_AX_OFLD_REQ_CLEANED) {
		if (state == MAC_AX_OFLD_REQ_H2C_SENT)
			return MACPROCERR;
	} else if (dest == MAC_AX_OFLD_REQ_CREATED) {
		if (state == MAC_AX_OFLD_REQ_IDLE ||
		    state == MAC_AX_OFLD_REQ_H2C_SENT)
			return MACPROCERR;
	} else if (dest == MAC_AX_OFLD_REQ_H2C_SENT) {
		if (state != MAC_AX_OFLD_REQ_CREATED)
			return MACPROCERR;
	}

	adapter->sm.read_request = dest;

	return MACSUCCESS;
}

u32 mac_clear_read_request(struct mac_ax_adapter *adapter)
{
	if (adapter->sm.read_request == MAC_AX_OFLD_REQ_H2C_SENT)
		return MACPROCERR;

	if (cnv_read_ofld_state(adapter, MAC_AX_OFLD_REQ_CLEANED)
	    != MACSUCCESS)
		return MACPROCERR;

	PLTFM_FREE(adapter->read_ofld_info.buf,
		   adapter->read_ofld_info.buf_size);
	adapter->read_ofld_info.buf = NULL;
	adapter->read_ofld_info.buf_wptr = NULL;
	adapter->read_ofld_info.last_req = NULL;
	adapter->read_ofld_info.buf_size = 0;
	adapter->read_ofld_info.avl_buf_size = 0;
	adapter->read_ofld_info.used_size = 0;
	adapter->read_ofld_info.req_num = 0;

	return MACSUCCESS;
}

u32 mac_add_read_request(struct mac_ax_adapter *adapter,
			 struct mac_ax_read_req *req)
{
	struct mac_ax_read_ofld_info *ofld_info = &adapter->read_ofld_info;
	struct fwcmd_read_ofld_req *write_ptr;
	u8 state;

	state = adapter->sm.read_request;

	if (!(state == MAC_AX_OFLD_REQ_CREATED ||
	      state == MAC_AX_OFLD_REQ_CLEANED)) {
		return MACPROCERR;
	}

	if (!ofld_info->buf) {
		ofld_info->buf = (u8 *)PLTFM_MALLOC(READ_OFLD_MAX_LEN);
		if (!ofld_info->buf)
			return MACNPTR;
		ofld_info->buf_wptr = ofld_info->buf;
		ofld_info->buf_size = READ_OFLD_MAX_LEN;
		ofld_info->avl_buf_size = READ_OFLD_MAX_LEN;
		ofld_info->used_size = 0;
		ofld_info->req_num = 0;
	}

	if (ofld_info->avl_buf_size < sizeof(struct mac_ax_read_req))
		return MACNOBUF;

	if (cnv_read_ofld_state(adapter, MAC_AX_OFLD_REQ_CREATED) != MACSUCCESS)
		return MACPROCERR;

	if (ofld_info->req_num != 0)
		ofld_info->last_req->ls = 0;

	ofld_info->last_req = (struct mac_ax_read_req *)ofld_info->buf_wptr;

	req->ls = 1;

	write_ptr = (struct fwcmd_read_ofld_req *)ofld_info->buf_wptr;
	write_ptr->dword0 =
	cpu_to_le32(SET_WORD(req->value_len,
			     FWCMD_H2C_READ_OFLD_REQ_VALUE_LEN) |
		    SET_WORD(req->ofld_id,
			     FWCMD_H2C_READ_OFLD_REQ_OFLD_ID) |
		    SET_WORD(req->entry_num,
			     FWCMD_H2C_READ_OFLD_REQ_ENTRY_NUM) | req->ls
	);

	write_ptr->dword1 =
	cpu_to_le32(SET_WORD(req->offset,
			     FWCMD_H2C_READ_OFLD_REQ_OFFSET)
	);

	ofld_info->buf_wptr += sizeof(struct mac_ax_read_req);
	ofld_info->avl_buf_size -= sizeof(struct mac_ax_read_req);
	ofld_info->used_size += sizeof(struct mac_ax_read_req);
	ofld_info->req_num++;

	return MACSUCCESS;
}

u32 mac_read_ofld(struct mac_ax_adapter *adapter)
{
	u8 *buf;
	u32 ret;
	#if MAC_AX_PHL_H2C
	struct rtw_h2c_pkt *h2cb;
	#else
	struct h2c_buf *h2cb;
	#endif
	struct mac_ax_read_ofld_info *ofld_info = &adapter->read_ofld_info;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY)
		return MACNOFW;

	if (ofld_info->used_size + FWCMD_HDR_LEN > READ_OFLD_MAX_LEN)
		return MACBUFSZ;

	if (adapter->sm.read_h2c != MAC_AX_OFLD_H2C_IDLE)
		return MACPROCERR;

	if (adapter->sm.read_request != MAC_AX_OFLD_REQ_CREATED)
		return MACPROCERR;

	if (cnv_read_ofld_state(adapter,
				MAC_AX_OFLD_REQ_H2C_SENT) != MACSUCCESS)
		return MACPROCERR;

	adapter->sm.read_h2c = MAC_AX_OFLD_H2C_SENDING;

	h2cb = h2cb_alloc(adapter, H2CB_CLASS_LONG_DATA);
	if (!h2cb)
		return MACNPTR;

	buf = h2cb_put(h2cb, ofld_info->used_size);
	if (!buf) {
		ret = MACNOBUF;
		goto fail;
	}

	PLTFM_MEMCPY(buf, ofld_info->buf, ofld_info->used_size);

	ret = h2c_pkt_set_hdr(adapter, h2cb,
			      FWCMD_TYPE_H2C, FWCMD_H2C_CAT_MAC,
			      FWCMD_H2C_CL_FW_OFLD, FWCMD_H2C_FUNC_READ_OFLD,
			      1, 1);

	if (ret)
		goto fail;

	ret = h2c_pkt_build_txd(adapter, h2cb);
	if (ret)
		goto fail;

	#if MAC_AX_PHL_H2C
	ret = PLTFM_TX(h2cb);
	#else
	ret = PLTFM_TX(h2cb->data, h2cb->len);
	#endif
	if (ret) {
		PLTFM_MSG_ERR("[ERR]platform tx: %d\n", ret);
		adapter->sm.read_request = MAC_AX_OFLD_REQ_IDLE;
		adapter->sm.read_h2c = MAC_AX_OFLD_H2C_IDLE;
		goto fail;
	}

	h2cb_free(adapter, h2cb);

	if (cnv_read_ofld_state(adapter, MAC_AX_OFLD_REQ_IDLE) != MACSUCCESS)
		return MACPROCERR;

	return MACSUCCESS;
fail:
	h2cb_free(adapter, h2cb);

	return ret;
}

u32 mac_read_ofld_value(struct mac_ax_adapter *adapter,
			u8 **val_buf, u16 *val_len)
{
	struct mac_ax_read_ofld_value *value_info = &adapter->read_ofld_value;
	*val_buf = NULL;

	if (adapter->sm.read_h2c != MAC_AX_OFLD_H2C_DONE)
		return MACPROCERR;

	*val_buf = (u8 *)PLTFM_MALLOC(value_info->len);
	if (!*val_buf)
		return MACBUFALLOC;

	PLTFM_MEMCPY(*val_buf, value_info->buf, value_info->len);

	*val_len = value_info->len;

	adapter->sm.read_h2c = MAC_AX_OFLD_H2C_IDLE;

	return MACSUCCESS;
}

u32 mac_general_pkt_ids(struct mac_ax_adapter *adapter,
			struct mac_ax_general_pkt_ids *ids)
{
	u8 *buf;
	u32 ret;
	#if MAC_AX_PHL_H2C
	struct rtw_h2c_pkt *h2cb;
	#else
	struct h2c_buf *h2cb;
	#endif
	struct fwcmd_general_pkt *write_ptr;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY)
		return MACNOFW;

	h2cb = h2cb_alloc(adapter, H2CB_CLASS_CMD);
	if (!h2cb)
		return MACNPTR;

	buf = h2cb_put(h2cb, sizeof(struct mac_ax_general_pkt_ids));
	if (!buf) {
		ret = MACNOBUF;
		goto fail;
	}

	write_ptr = (struct fwcmd_general_pkt *)buf;
	write_ptr->dword0 =
	cpu_to_le32(SET_WORD(ids->macid, FWCMD_H2C_GENERAL_PKT_MACID) |
		    SET_WORD(ids->probersp, FWCMD_H2C_GENERAL_PKT_PROBRSP_ID) |
		    SET_WORD(ids->pspoll, FWCMD_H2C_GENERAL_PKT_PSPOLL_ID) |
		    SET_WORD(ids->nulldata, FWCMD_H2C_GENERAL_PKT_NULL_ID)
	);

	write_ptr->dword1 =
	cpu_to_le32(SET_WORD(ids->qosnull, FWCMD_H2C_GENERAL_PKT_QOS_NULL_ID) |
		    SET_WORD(ids->cts2self, FWCMD_H2C_GENERAL_PKT_CTS2SELF_ID) |
		    SET_WORD(ids->probereq, FWCMD_H2C_GENERAL_PKT_PROBREQ_ID) |
		    SET_WORD(ids->apcsa, FWCMD_H2C_GENERAL_PKT_APCSA_ID)
	);

	ret = h2c_pkt_set_hdr(adapter, h2cb,
			      FWCMD_TYPE_H2C, FWCMD_H2C_CAT_MAC,
			      FWCMD_H2C_CL_FW_INFO,
			      FWCMD_H2C_FUNC_GENERAL_PKT,
			      1, 1);
	if (ret)
		goto fail;

	ret = h2c_pkt_build_txd(adapter, h2cb);
	if (ret)
		goto fail;

	#if MAC_AX_PHL_H2C
	ret = PLTFM_TX(h2cb);
	#else
	ret = PLTFM_TX(h2cb->data, h2cb->len);
	#endif
	if (ret) {
		PLTFM_MSG_ERR("[ERR]platform tx: %d\n", ret);
		goto fail;
	}

	h2cb_free(adapter, h2cb);

	return MACSUCCESS;
fail:
	h2cb_free(adapter, h2cb);

	return ret;
}

static u32 add_cmd(struct mac_ax_adapter *adapter, struct rtw_mac_cmd *cmd)
{
	struct mac_ax_cmd_ofld_info *ofld_info = &adapter->cmd_ofld_info;
	u16 total_len = CMD_OFLD_SIZE;
	struct fwcmd_cmd_ofld *write_ptr;

	if (!ofld_info->buf) {
		ofld_info->buf = (u8 *)PLTFM_MALLOC(CMD_OFLD_MAX_LEN);
		if (!ofld_info->buf)
			return MACBUFALLOC;
		ofld_info->buf_wptr = ofld_info->buf;
		ofld_info->last_wptr = NULL;
		ofld_info->buf_size = CMD_OFLD_MAX_LEN;
		ofld_info->avl_buf_size = CMD_OFLD_MAX_LEN;
		ofld_info->used_size = 0;
		ofld_info->cmd_num = 0;
		ofld_info->accu_delay = 0;
	}

	write_ptr = (struct fwcmd_cmd_ofld *)ofld_info->buf_wptr;

	write_ptr->dword0 =
	cpu_to_le32(SET_WORD(cmd->src, FWCMD_H2C_CMD_OFLD_SRC) |
		    SET_WORD(cmd->type, FWCMD_H2C_CMD_OFLD_TYPE) |
		    (cmd->lc ? FWCMD_H2C_CMD_OFLD_LC : 0) |
		    SET_WORD(cmd->rf_path, FWCMD_H2C_CMD_OFLD_PATH) |
		    SET_WORD(cmd->offset, FWCMD_H2C_CMD_OFLD_OFFSET) |
		    SET_WORD(ofld_info->cmd_num, FWCMD_H2C_CMD_OFLD_CMD_NUM)
	);
	write_ptr->dword1 =
	cpu_to_le32(SET_WORD(cmd->id, FWCMD_H2C_CMD_OFLD_ID));
	write_ptr->dword2 =
	cpu_to_le32(SET_WORD(cmd->value, FWCMD_H2C_CMD_OFLD_VALUE));
	write_ptr->dword3 =
	cpu_to_le32(SET_WORD(cmd->mask, FWCMD_H2C_CMD_OFLD_MASK));

	ofld_info->last_wptr = ofld_info->buf_wptr;
	ofld_info->buf_wptr += total_len;
	ofld_info->avl_buf_size -= total_len;
	ofld_info->used_size += total_len;
	ofld_info->cmd_num++;
	if (cmd->type == RTW_MAC_DELAY_OFLD)
		ofld_info->accu_delay += cmd->value;

	return MACSUCCESS;
}

static u32 chk_cmd_ofld_reg(struct mac_ax_adapter *adapter)
{
#define MAC_AX_CMD_OFLD_POLL_CNT 1000
#define MAC_AX_CMD_OFLD_POLL_US 50
	struct mac_ax_c2hreg_poll c2h;
	struct fwcmd_c2hreg *c2h_content;
	u32 ret, result, i, cmd_num;
	struct mac_ax_cmd_ofld_info *ofld_info = &adapter->cmd_ofld_info;
	u8 *cmd;

	c2h.polling_id = FWCMD_C2HREG_FUNC_IO_OFLD_RESULT;
	c2h.retry_cnt = MAC_AX_CMD_OFLD_POLL_CNT;
	c2h.retry_wait_us = MAC_AX_CMD_OFLD_POLL_US;
	ret = proc_msg_reg(adapter, NULL, &c2h);
	if (ret) {
		PLTFM_MSG_ERR("%s: fail to wait FW done(%d)\n", __func__, ret);
		return ret;
	}

	c2h_content = &c2h.c2hreg_cont.c2h_content;
	result = GET_FIELD(c2h_content->dword0,
			   FWCMD_C2HREG_IO_OFLD_RESULT_RET);
	if (result) {
		cmd_num = GET_FIELD(c2h_content->dword0,
				    FWCMD_C2HREG_IO_OFLD_RESULT_CMD_NUM);
		cmd = ofld_info->buf + cmd_num * CMD_OFLD_SIZE;
		PLTFM_MSG_ERR("%s: fail to finish IO offload\n", __func__);
		PLTFM_MSG_ERR("fail offset = %x\n", c2h_content->dword1);
		PLTFM_MSG_ERR("exp val = %x\n", c2h_content->dword2);
		PLTFM_MSG_ERR("read val = %x\n", c2h_content->dword3);
		PLTFM_MSG_ERR("fail cmd num = %d\n", cmd_num);
		for (i = 0; i < CMD_OFLD_SIZE; i += 4)
			PLTFM_MSG_ERR("%x\n", *((u32 *)(cmd + i)));

		return MACFIOOFLD;
	}

	return MACSUCCESS;
}

static u32 chk_cmd_ofld_pkt(struct mac_ax_adapter *adapter)
{
	u32 cnt = MAC_AX_CMD_OFLD_POLL_CNT;
	struct mac_ax_state_mach *sm = &adapter->sm;
	struct mac_ax_drv_stats *drv_stats = &adapter->drv_stats;
	struct mac_ax_cmd_ofld_info *ofld_info = &adapter->cmd_ofld_info;

	while (--cnt) {
		if (sm->cmd_state == MAC_AX_CMD_OFLD_RCVD)
			break;
		if (drv_stats->drv_rm)
			return MACDRVRM;
		PLTFM_DELAY_US(MAC_AX_CMD_OFLD_POLL_US);
	}

	PLTFM_MSG_TRACE("%s: cnt = %d, us = %d\n",
			__func__, cnt, MAC_AX_CMD_OFLD_POLL_US);

	if (!cnt) {
		PLTFM_MSG_ERR("%s: polling timeout\n", __func__);
		return MACPOLLTO;
	}

	if (ofld_info->result) {
		PLTFM_MSG_ERR("%s: ofld FAIL!!!\n", __func__);
		return MACFIOOFLD;
	}

	return MACSUCCESS;
}

static u32 chk_cmd_ofld(struct mac_ax_adapter *adapter, u8 rx_ok)
{
	u32 ret;

	if (rx_ok)
		ret = chk_cmd_ofld_pkt(adapter);
	else
		ret = chk_cmd_ofld_reg(adapter);

	return ret;
}

static u32 cmd_ofld(struct mac_ax_adapter *adapter)
{
	u32 ret;
	#if MAC_AX_PHL_H2C
	struct rtw_h2c_pkt *h2cb;
	#else
	struct h2c_buf *h2cb;
	#endif
	struct mac_ax_cmd_ofld_info *ofld_info = &adapter->cmd_ofld_info;
	u8 *buffer;
	u8 func;
	u8 rx_ok = adapter->drv_stats.rx_ok;
	struct mac_ax_state_mach *sm = &adapter->sm;

	PLTFM_MSG_TRACE("%s===>\n", __func__);

	h2cb = h2cb_alloc(adapter, H2CB_CLASS_LONG_DATA);
	if (!h2cb)
		return MACNPTR;

	buffer = h2cb_put(h2cb, ofld_info->used_size);
	if (!buffer) {
		ret = MACNOBUF;
		goto fail;
	}

	PLTFM_MEMCPY(buffer, ofld_info->buf, ofld_info->used_size);

	func = rx_ok ? FWCMD_H2C_FUNC_CMD_OFLD_PKT :
		FWCMD_H2C_FUNC_CMD_OFLD_REG;

	ret = h2c_pkt_set_hdr(adapter, h2cb,
			      FWCMD_TYPE_H2C, FWCMD_H2C_CAT_MAC,
			      FWCMD_H2C_CL_FW_OFLD, func,
			      0, 0);
	if (ret)
		goto fail;

	ret = h2c_pkt_build_txd(adapter, h2cb);
	if (ret)
		goto fail;

	#if MAC_AX_PHL_H2C
	ret = PLTFM_TX(h2cb);
	#else
	ret = PLTFM_TX(h2cb->data, h2cb->len);
	#endif
	if (ret) {
		PLTFM_MSG_ERR("[ERR]platform tx\n");
		goto fail;
	}

	if (ofld_info->accu_delay)
		PLTFM_DELAY_US(ofld_info->accu_delay);

	sm->cmd_state = MAC_AX_CMD_OFLD_SENDING;

	ret = chk_cmd_ofld(adapter, rx_ok);
	if (ret) {
		PLTFM_MSG_ERR("%s: check IO offload fail\n", __func__);
		goto fail;
	}

	h2cb_free(adapter, h2cb);
	PLTFM_FREE(ofld_info->buf, CMD_OFLD_MAX_LEN);
	ofld_info->buf = NULL;

	PLTFM_MSG_TRACE("%s<===\n", __func__);

	return MACSUCCESS;
fail:
	h2cb_free(adapter, h2cb);
	PLTFM_FREE(ofld_info->buf, CMD_OFLD_MAX_LEN);
	ofld_info->buf = NULL;

	return ret;
}

u32 mac_add_cmd_ofld(struct mac_ax_adapter *adapter, struct rtw_mac_cmd *cmd)
{
	struct mac_ax_cmd_ofld_info *ofld_info = &adapter->cmd_ofld_info;
	struct mac_ax_state_mach *sm = &adapter->sm;
	u32 ret = MACSUCCESS;

	if (cmd->type !=  RTW_MAC_DELAY_OFLD &&
	    cmd->src != RTW_MAC_RF_CMD_OFLD && cmd->offset & (4 - 1))
		return MACBADDR;

	if (adapter->sm.fwdl != MAC_AX_FWDL_INIT_RDY)
		return MACNOFW;

	PLTFM_MUTEX_LOCK(&ofld_info->cmd_ofld_lock);
	if (sm->cmd_state != MAC_AX_CMD_OFLD_IDLE) {
		PLTFM_MSG_ERR("%s: IO offload is busy\n", __func__);
		PLTFM_MUTEX_UNLOCK(&ofld_info->cmd_ofld_lock);
		return MACPROCERR;
	}
	sm->cmd_state = MAC_AX_CMD_OFLD_PROC;
	PLTFM_MUTEX_UNLOCK(&ofld_info->cmd_ofld_lock);

	if (ofld_info->buf &&
	    ofld_info->avl_buf_size < CMD_OFLD_SIZE) {
		if (!ofld_info->last_wptr) {
			ret = MACNPTR;
			PLTFM_MSG_ERR("%s: wrong pointer\n", __func__);
			goto END;
		}
		*ofld_info->last_wptr = *ofld_info->last_wptr |
			FWCMD_H2C_CMD_OFLD_LC;
		ret = cmd_ofld(adapter);
		if (ret) {
			PLTFM_MSG_ERR("%s: send IO offload fail\n", __func__);
			goto END;
		}
	}

	ret = add_cmd(adapter, cmd);
	if (ret)
		goto END;

	if (!cmd->lc)
		goto END;

	ret = cmd_ofld(adapter);

END:
	PLTFM_MUTEX_LOCK(&ofld_info->cmd_ofld_lock);
	sm->cmd_state = MAC_AX_CMD_OFLD_IDLE;
	PLTFM_MUTEX_UNLOCK(&ofld_info->cmd_ofld_lock);

	return ret;
}

u32 write_mac_reg_ofld(struct mac_ax_adapter *adapter,
		       u16 offset, u32 mask, u32 val, u8 lc)
{
	struct rtw_mac_cmd cmd = {RTW_MAC_MAC_CMD_OFLD, RTW_MAC_WRITE_OFLD,
		0, RTW_MAC_RF_PATH_A, 0, 0, 0, 0};

	cmd.offset = offset;
	cmd.mask = mask;
	cmd.value = val;
	cmd.lc = lc;

	return mac_add_cmd_ofld(adapter, &cmd);
}

u32 poll_mac_reg_ofld(struct mac_ax_adapter *adapter,
		      u16 offset, u32 mask, u32 val, u8 lc)
{
	struct rtw_mac_cmd cmd = {RTW_MAC_MAC_CMD_OFLD, RTW_MAC_COMPARE_OFLD,
		0, RTW_MAC_RF_PATH_A, 0, 0, 0, 0};

	cmd.offset = offset;
	cmd.mask = mask;
	cmd.value = val;
	cmd.lc = lc;

	return mac_add_cmd_ofld(adapter, &cmd);
}

u32 delay_ofld(struct mac_ax_adapter *adapter,
	       u32 val)
{
	struct rtw_mac_cmd cmd = {RTW_MAC_MAC_CMD_OFLD, RTW_MAC_DELAY_OFLD,
		0, RTW_MAC_RF_PATH_A, 0, 0, 0, 0};

	cmd.value = val;

	return mac_add_cmd_ofld(adapter, &cmd);
}

u32 mac_ccxrpt_parsing(struct mac_ax_adapter *adapter, u8 *buf, struct mac_ax_ccxrpt *info)
{
	u32 val_d0;
	u32 val_d3;
	u32 dword0 = *((u32 *)buf);
	u32 dword3 = *((u32 *)(buf + 12));

	val_d0 = le32_to_cpu(dword0);
	val_d3 = le32_to_cpu(dword3);
	info->tx_state = GET_FIELD(val_d0, TXCCXRPT_TX_STATE);
	info->sw_define = GET_FIELD(val_d0, TXCCXRPT_SW_DEFINE);
	info->macid = GET_FIELD(val_d0, TXCCXRPT_MACID);
	info->pkt_ok_num = GET_FIELD(val_d3, TXCCXRPT_PKT_OK_NUM);
	info->data_txcnt = GET_FIELD(val_d3, TXCCXRPT_DATA_TX_CNT);

	return MACSUCCESS;
}

u32 get_ccxrpt_event(struct mac_ax_adapter *adapter,
		     struct rtw_c2h_info *c2h,
		     enum phl_msg_evt_id *id, u8 *c2h_info)
{
	struct mac_ax_ccxrpt *info;
	u32 val_d0, val_d3;
	u32 dword0 = *((u32 *)c2h->content);
	u32 dword3 = *((u32 *)(c2h->content + 12));

	info = (struct mac_ax_ccxrpt *)c2h_info;
	val_d0 = le32_to_cpu(dword0);
	val_d3 = le32_to_cpu(dword3);
	info->tx_state = GET_FIELD(val_d0, TXCCXRPT_TX_STATE);
	info->sw_define = GET_FIELD(val_d0, TXCCXRPT_SW_DEFINE);
	info->macid = GET_FIELD(val_d0, TXCCXRPT_MACID);
	info->pkt_ok_num = GET_FIELD(val_d3, TXCCXRPT_PKT_OK_NUM);
	info->data_txcnt = GET_FIELD(val_d3, TXCCXRPT_DATA_TX_CNT);

	if (info->tx_state)
		*id = MSG_EVT_CCX_REPORT_TX_FAIL;
	else
		*id = MSG_EVT_CCX_REPORT_TX_OK;

	return MACSUCCESS;
}
