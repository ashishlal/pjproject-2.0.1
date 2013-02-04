/* $Id: h263_packetizer.c 4006 2012-04-02 08:40:54Z nanang $ */
/* 
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */
#include <pjmedia-codec/h263_packetizer.h>
#include <pjmedia/types.h>
#include <pj/assert.h>
#include <pj/errno.h>
#include <pj/string.h>
#include <pj/list.h>
#include <pj/math.h>

#if defined(PJMEDIA_HAS_VIDEO) && (PJMEDIA_HAS_VIDEO != 0)


#define THIS_FILE	"h263_packetizer.c"

#if 0
/* H.263 packetizer definition */
struct pjmedia_h263_packetizer {
    /* Current settings */
    pjmedia_h263_packetizer_cfg cfg;
    
    /* Unpacketizer state */
    unsigned	    unpack_last_sync_pos;
    pj_bool_t	    unpack_prev_lost;

    int width;
	int height;
	unsigned pict_type;
};
#endif

PJ_DEF(pj_status_t) pjmedia_h263_1998_packetize(pjmedia_h263_packetizer *pktz,
    pj_uint8_t *bits,
    pj_size_t bits_len,
    unsigned *pos,
    const pj_uint8_t **payload,
	pj_size_t *payload_len);

PJ_DEF(pj_status_t) pjmedia_h263_1998_unpacketize (pjmedia_h263_packetizer *pktz,
												const pj_uint8_t *payload,
												pj_size_t payload_len,
												pj_uint8_t *bits,
												pj_size_t bits_size,
												unsigned *pos);
PJ_DEF(pj_status_t) pjmedia_h263_1996_packetize(pjmedia_h263_packetizer *pktz,
									pj_uint8_t *bits,
									pj_size_t bits_len,
									unsigned *pos,
									pj_uint8_t **payload,
									pj_size_t *payload_len);
										
PJ_DEF(pj_status_t) pjmedia_h263_1996_unpacketize (pjmedia_h263_packetizer *pktz,
			const pj_uint8_t *payload,
			pj_size_t payload_len,
			pj_uint8_t *bits,
			pj_size_t bits_size,
			unsigned *pos);
			 

static int get_gbsc_bytealigned(pj_uint8_t *begin, pj_uint8_t *end){
        int i;
        int len = end - begin;
        for (i = len - 2;  /*len + length of scan window*/
           i > 2 + 2; /*length of scan window + 2 avoidance of 1st gob or psc*/
           i--){
                if(*(begin + i) == 0 &&
                   *(begin + i+1) == 0 &&
                   (*(begin + i+2) & 0x80) == 0x80){
                  /*printf("JV psc/gob found! %2x %2x %2x", *(begin + i), *(begin + i+1), *(begin + i + 2));*/
                  return i;
                }
        }
        /*printf("JV no psc or gob found!");*/
        return len;
}

/*
 * Find synchronization point (PSC, slice, GSBC, EOS, EOSBS) in H.263 
 * bitstream.
 */
static pj_uint8_t* find_sync_point(pj_uint8_t *data,
				   pj_size_t data_len)
{
    pj_uint8_t *p = data, *end = data+data_len-1;

    while (p < end && (*p || *(p+1)))
        ++p;

    if (p == end)
        return NULL;
        
    return p;
}


/*
 * Find synchronization point (PSC, slice, GSBC, EOS, EOSBS) in H.263 
 * bitstream, in reversed manner.
 */
static pj_uint8_t* find_sync_point_rev(pj_uint8_t *data,
                                       pj_size_t data_len)
{
    pj_uint8_t *p = data+data_len-2;

    while (p >= data && (*p || *(p+1)))
        --p;

    if (p < data)
        return (data + data_len);
        
    return p;
}


/*
 * Create H263 packetizer.
 */
PJ_DEF(pj_status_t) pjmedia_h263_packetizer_create(
				pj_pool_t *pool,
				const pjmedia_h263_packetizer_cfg *cfg,
				pjmedia_h263_packetizer **p)
{
    pjmedia_h263_packetizer *p_;

    PJ_ASSERT_RETURN(pool && p, PJ_EINVAL);

    //if (cfg && (cfg->mode != PJMEDIA_H263_PACKETIZER_MODE_RFC4629 && cfg->mode != PJMEDIA_H263_PACKETIZER_MODE_RFC2190))
	//    return PJ_ENOTSUP;
	if(!cfg) return PJ_ENOTSUP;
    p_ = PJ_POOL_ZALLOC_T(pool, pjmedia_h263_packetizer);
    if (cfg) {
	    pj_memcpy(&p_->cfg, cfg, sizeof(*cfg));
	    if(cfg->mode == PJMEDIA_H263_PACKETIZER_MODE_RFC2190) {
			p_->cfg.mtu = PJMEDIA_MAX_VID_PAYLOAD_SIZE;
#define H263_MAX_ENCODE_SIZE 20000
			p_->max_encoded_size = H263_MAX_ENCODE_SIZE;
			p_->pktOrig= (unsigned *)malloc(p_->max_encoded_size+64+4);
			unsigned  *r = (unsigned *)(((unsigned)(p_->pktOrig) + 64) & (~63));
			p_->packet = (pj_uint8_t *)r;
		}
    } else {
	p_->cfg.mode = PJMEDIA_H263_PACKETIZER_MODE_RFC4629;
	p_->cfg.mtu = PJMEDIA_MAX_VID_PAYLOAD_SIZE;
    }

    *p = p_;

    return PJ_SUCCESS;
}


/*
 * Generate an RTP payload from H.263 frame bitstream, in-place processing.
 */
PJ_DEF(pj_status_t) pjmedia_h263_packetize(pjmedia_h263_packetizer *pktz,
                                           pj_uint8_t *bits,
                                           pj_size_t bits_len,
                                           unsigned *pos,
                                           const pj_uint8_t **payload,
                                           pj_size_t *payload_len) {
    if (pktz->cfg.mode == PJMEDIA_H263_PACKETIZER_MODE_RFC4629) {
        return pjmedia_h263_1998_packetize(pktz, bits, bits_len, pos, payload, payload_len);
    } else if (pktz->cfg.mode == PJMEDIA_H263_PACKETIZER_MODE_RFC2190) {
        return pjmedia_h263_1996_packetize(pktz, bits, bits_len, pos, (pj_uint8_t **)payload, payload_len);
    }
	return PJ_SUCCESS;
}

/*
 * Generate an RTP payload from H.263 frame bitstream, in-place processing.
 */
PJ_DEF(pj_status_t) pjmedia_h263_1998_packetize(pjmedia_h263_packetizer *pktz,
					   pj_uint8_t *bits,
                                           pj_size_t bits_len,
                                           unsigned *pos,
                                           const pj_uint8_t **payload,
                                           pj_size_t *payload_len)
{
    pj_uint8_t *p, *end;

    pj_assert(pktz && bits && pos && payload && payload_len);
    pj_assert(*pos <= bits_len);

    p = bits + *pos;
    end = bits + bits_len;

    //printf_hex_with_text("[pjmedia_h263_1998_packetize] bitstream", bits, bits_len);
    /* Put two octets payload header */
    if ((end-p > 2) && *p==0 && *(p+1)==0) {
        /* The bitstream starts with synchronization point, just override
         * the two zero octets (sync point mark) for payload header.
         */
        *p = 0x04;
    } else {
        /* Not started in synchronization point, we will use two octets
         * preceeding the bitstream for payload header!
         */

	if (*pos < 2) {
	    /* Invalid H263 bitstream, it's not started with PSC */
	    return PJ_EINVAL;
	}

	p -= 2;
        *p = 0;
    }
    *(p+1) = 0;

    /* When bitstream truncation needed because of payload length/MTU 
     * limitation, try to use sync point for the payload boundary.
     */
    if (end-p > pktz->cfg.mtu) {
	end = find_sync_point_rev(p+2, pktz->cfg.mtu-2);
    }

    *payload = p;
    *payload_len = end-p;
    *pos = end - bits;

    //printf_hex_with_text("[pjmedia_h263_1998_packetize] payload", *payload, *payload_len);
    
    return PJ_SUCCESS;
}
/* Hieu add */

/*
 * Append an RTP payload to a H.263 picture bitstream.
 */
PJ_DEF(pj_status_t) pjmedia_h263_unpacketize (pjmedia_h263_packetizer *pktz,
                                              const pj_uint8_t *payload,
                                              pj_size_t payload_len,
                                              pj_uint8_t *bits,
                                              pj_size_t bits_size,
                                              unsigned *pos) {
    if (pktz->cfg.mode == PJMEDIA_H263_PACKETIZER_MODE_RFC4629) {
        return pjmedia_h263_1998_unpacketize(pktz, payload, payload_len, bits, bits_size, pos);
    } else if (pktz->cfg.mode == PJMEDIA_H263_PACKETIZER_MODE_RFC2190) {
        return pjmedia_h263_1996_unpacketize(pktz, payload, payload_len, bits, bits_size, pos);
    }
	return PJ_SUCCESS;
}

/* End Hieu adding */

/*
 * Append an RTP payload to a H.263 picture bitstream.
 */
PJ_DEF(pj_status_t) pjmedia_h263_1998_unpacketize (pjmedia_h263_packetizer *pktz,
					      const pj_uint8_t *payload,
                          pj_size_t payload_len,
                          pj_uint8_t *bits,
                          pj_size_t bits_size,
					      unsigned *pos)
{
    pj_uint8_t P, V, PLEN;
    const pj_uint8_t *p = payload;
    pj_uint8_t *q;

    q = bits + *pos;

   // printf("-------------%s: Inside pjmedia_h263_1998_unpacketize-------------\n", THIS_FILE);
    /* Check if this is a missing/lost packet */
    if (payload == NULL) {
	pktz->unpack_prev_lost = PJ_TRUE;
	return PJ_SUCCESS;
    }

    //printf_hex_with_text("[pjmedia_h263_1998_unpacketize] payload", payload, payload_len);
    /* H263 payload header size is two octets */
    if (payload_len < 2) {
	/* Invalid bitstream, discard this payload */
	pktz->unpack_prev_lost = PJ_TRUE;
	return PJ_EINVAL;
    }

    /* Reset last sync point for every new picture bitstream */
    if (*pos == 0)
	pktz->unpack_last_sync_pos = 0;

    /* Get payload header info */
    P = *p & 0x04;
    V = *p & 0x02;
    PLEN = ((*p & 0x01) << 5) + ((*(p+1) & 0xF8)>>3);

    /* Get start bitstream pointer */
    p += 2;	    /* Skip payload header */
    if (V)
        p += 1;	    /* Skip VRC data */
    if (PLEN)
        p += PLEN;  /* Skip extra picture header data */

    /* Get bitstream length */
    if (payload_len > (pj_size_t)(p - payload)) {
	payload_len -= (p - payload);
    } else {
	/* Invalid bitstream, discard this payload */
	pktz->unpack_prev_lost = PJ_TRUE;
	return PJ_EINVAL;
    }

    /* Validate bitstream length */
    if (bits_size < *pos + payload_len + 2) {
	/* Insufficient bistream buffer, discard this payload */
	pj_assert(!"Insufficient H.263 bitstream buffer");
	pktz->unpack_prev_lost = PJ_TRUE;
	return PJ_ETOOSMALL;
    }

    /* Start writing bitstream */

    /* No sync point flag */
    if (!P) {
	if (*pos == 0) {
	    /* Previous packet must be lost */
	    pktz->unpack_prev_lost = PJ_TRUE;

	    /* If there is extra picture header, let's use it. */
	    if (PLEN) {
		/* Write two zero octets for PSC */
		*q++ = 0;
		*q++ = 0;
		/* Copy the picture header */
		p -= PLEN;
		pj_memcpy(q, p, PLEN);
		p += PLEN;
		q += PLEN;
	    }
	} else if (pktz->unpack_prev_lost) {
	    /* If prev packet was lost, revert the bitstream pointer to
	     * the last sync point.
	     */
	    pj_assert(pktz->unpack_last_sync_pos <= *pos);
	    q = bits + pktz->unpack_last_sync_pos;
	}

	/* There was packet lost, see if this payload contain sync point
	 * (usable data).
	 */
	if (pktz->unpack_prev_lost) {
	    pj_uint8_t *sync;
	    sync = find_sync_point((pj_uint8_t*)p, payload_len);
	    if (sync) {
		/* Got sync point, update P/sync-point flag */
		P = 1;
		/* Skip the two zero octets */
		sync += 2;
		/* Update payload length and start bitstream pointer */
		payload_len -= (sync - p);
		p = sync;
	    } else {
		/* No sync point in it, just discard this payload */
		return PJ_EIGNORED;
	    }
	}
    }

    /* Write two zero octets when payload flagged with sync point */
    if (P) {
	pktz->unpack_last_sync_pos = q - bits;
        *q++ = 0;
        *q++ = 0;
    }

    /* Write the payload to the bitstream */
    pj_memcpy(q, p, payload_len);
    q += payload_len;

    /* Update the bitstream writing offset */
    *pos = q - bits;

    pktz->unpack_prev_lost = PJ_FALSE;

    //printf_hex_with_text("[pjmedia_h263_1998_unpacketize] bitstream", bits, bits_size);
    
    return PJ_SUCCESS;
}

/* Hieu add */
PJ_DEF(pj_status_t) pjmedia_h263_1996_packetize(pjmedia_h263_packetizer *pktz,
                                                pj_uint8_t *bits,
                                                pj_size_t bits_len,
                                                unsigned *pos,
                                                pj_uint8_t **payload,
                                                pj_size_t *payload_len)
{
	pj_uint8_t *p, *end;

    pj_assert(pktz && bits && pos && payload && payload_len);
    pj_assert(*pos <= bits_len);
	unsigned flags=0;
	
    p = bits + *pos;
    end = bits + bits_len;
	
	//printf("------------------%s: Inside pjmedia_h263_1996_packetize---------------, %d\n", THIS_FILE, (int)(end-p));

	if(*pos == 0) {
		if(bits_len < pktz->max_encoded_size) {
	        memcpy(pktz->packet, bits, bits_len);
        }
        else {
			pktz->max_encoded_size = bits_len;
			pktz->pktOrig = (unsigned *)realloc(pktz->pktOrig, pktz->max_encoded_size+64+4);
			unsigned  *r = (unsigned *)(((unsigned)(pktz->pktOrig) + 64) & (~63));
			pktz->packet = (pj_uint8_t *)r;
			memcpy(pktz->packet, bits, bits_len);
        }
	}
	const unsigned char * data = pktz->packet + *pos;
    {
	    unsigned len = get_gbsc_bytealigned((pj_uint8_t *)data, 
	                   (pj_uint8_t *)PJ_MIN(((unsigned)(data))+pktz->cfg.mtu,((unsigned)(pktz->packet))+bits_len));
	    unsigned char header[4];
		memset(header, 0, 4);
		int iFrame = ((data[4] & 2) == 0);
#ifndef MS_LYNC
        // assume video size is CIF or QCIF
        if (pktz->width == 352 && pktz->height == 288) 
            header[1] = 0x60;
        else header[1] = 0x40;
		if(!iFrame) header[1] |= 0x10;
#else
        // Assume CIF only for MS-Lync
        if(!iFrame {
			header[0] = 0x05;
			header[1] = 0x60;
	    }
	    else {
		    header[0] = 0x02;
			header[1] = 0x70;
	    }
#endif
        *payload = p;
		memcpy(p, header, 4);
		p += 4;
		memcpy(p, data, len);
		p += len;
		*payload_len = (4 + len);
		unsigned pos1 = *pos;
		*pos = (p - bits-4) < bits_len?(p-bits-4):bits_len;
		//printf("bits: 0x%02x, p: 0x%02x, payload_len: %d, pos1: %d, pos: %d, bits_len: %d, iFrame:%d\n", 
		//    bits, p, *payload_len, pos1, *pos, bits_len, pktz->pict_type);
	}

    return PJ_SUCCESS;
}

PJ_DEF(pj_status_t) pjmedia_h263_1996_unpacketize (pjmedia_h263_packetizer *pktz,
                                                   const pj_uint8_t *payload,
                                                   pj_size_t payload_len,
                                                   pj_uint8_t *bits,
                                                   pj_size_t bits_size,
                                                   unsigned *pos)
{
	unsigned char mode; 
	unsigned hdrLen;
	int iFrame=0;
    const pj_uint8_t *p = payload;
    pj_uint8_t *q;
	
    q = bits + *pos;

   // printf("-------------%s: Inside pjmedia_h263_1996_unpacketize-------------\n", THIS_FILE);
    /* Check if this is a missing/lost packet */
    if (payload == NULL) {
	    pktz->unpack_prev_lost = PJ_TRUE;
	     return PJ_SUCCESS;
    }

    /* H263-1996 payload header size is atleast 4 octets */
    if (payload_len < 4) {
	    /* Invalid bitstream, discard this payload */
	    pktz->unpack_prev_lost = PJ_TRUE;
	    return PJ_EINVAL;
    }

    /* Reset last sync point for every new picture bitstream */
    if (*pos == 0)
	    pktz->unpack_last_sync_pos = 0;
#ifndef MS_LYNC
	if ((p[0] & 0x80) == 0) {
		iFrame = ((p[1] & 0x10) == 0);
	    hdrLen = 4;
	    mode = 'A';
	} 
	else if ((p[0] & 0x40) == 0) {
		iFrame = ((p[4] & 0x80) == 0);
	    hdrLen = 8;
	    mode = 'B';
	} 
	else {
	    iFrame = ((p[4] & 0x80) == 0);
	    hdrLen = 12;
	    mode = 'C';
	}
#else
    if ((p[0] == 0x05) && (p[1] == 0x60) && (p[2] == 0) && (p[3] == 0)) {
		iFrame = 0;
	    hdrLen = 4;
	    mode = 'A';
	} 
	else if ((p[0] == 0x02) && (p[1] == 0x70) && (p[2] == 0) && (p[3] == 0)) {
		iFrame = 1;
	    hdrLen = 4;
	    mode = 'A';
	} 
	else if ((p[0] == 0xbd) && (p[1] == 0x67) && (p[2] == 0) && (p[3] == 0x14)
	         && (p[4] == 0) && (p[5] == 0) && (p[6] == 0) && (p[7] == 0)) {
		iFrame = 0;
		hdrLen = 8;
		mode = 'B';
	}
	else if ((p[0] == 0xa1) && (p[1] == 0x67) && (p[2] == 0) && (p[3] == 0x18)
	         && (p[4] == 0x8f) && (p[5] == 0) && (p[6] == 0x80) && (p[7] == 0)) {
		iFrame = 1;
		hdrLen = 8;
		mode = 'B';
	}
	else {
	    iFrame = 0;
	    hdrLen = 12;
	    mode = 'C';
	}
#endif
	/* H263-1996 payload header size is more than hdrLen */
    if (payload_len < hdrLen) {
	    /* Invalid bitstream, discard this payload */
	    pktz->unpack_prev_lost = PJ_TRUE;
	    return PJ_EINVAL;
    }

    /* Reset last sync point for every new picture bitstream */
    if (*pos == 0)
	    pktz->unpack_last_sync_pos = 0;

    /* Get bitstream length */
	p += hdrLen;
    if (payload_len > (pj_size_t)(p - payload)) {
	    payload_len -= (p - payload);
    } else {
	    /* Invalid bitstream, discard this payload */
	    pktz->unpack_prev_lost = PJ_TRUE;
	    return PJ_EINVAL;
    }

    /* Validate bitstream length */
    if (bits_size < *pos + payload_len + hdrLen) {
	    /* Insufficient bistream buffer, discard this payload */
	    pj_assert(!"Insufficient H.263 bitstream buffer");
	    pktz->unpack_prev_lost = PJ_TRUE;
	    return PJ_ETOOSMALL;
    }
	/* Write the payload to the bitstream */
    pj_memcpy(q, p, payload_len);
    q += payload_len;

    /* Update the bitstream writing offset */
    *pos = q - bits;

    //printf("Unpack: bits: 0x%02x, p: 0x%02x, payload_len: %d, hdrLen: %d, pos: %d, bits_len: %d, iFrame:%d\n", 
	//    bits, p, payload_len, hdrLen, *pos, bits_size, iFrame);
	
    pktz->unpack_prev_lost = PJ_FALSE;
	
}
/* End Hieu adding */


#endif /* PJMEDIA_HAS_VIDEO */
