/*
 * Copyright 2002-2003. The Regents of the University of California. This material 
 * was produced under U.S. Government contract W-7405-ENG-36 for Los Alamos 
 * National Laboratory, which is operated by the University of California for 
 * the U.S. Department of Energy. The Government is granted for itself and 
 * others acting on its behalf a paid-up, nonexclusive, irrevocable worldwide 
 * license in this material to reproduce, prepare derivative works, and 
 * perform publicly and display publicly. Beginning five (5) years after 
 * October 10,2002 subject to additional five-year worldwide renewals, the 
 * Government is granted for itself and others acting on its behalf a paid-up, 
 * nonexclusive, irrevocable worldwide license in this material to reproduce, 
 * prepare derivative works, distribute copies to the public, perform publicly 
 * and display publicly, and to permit others to do so. NEITHER THE UNITED 
 * STATES NOR THE UNITED STATES DEPARTMENT OF ENERGY, NOR THE UNIVERSITY OF 
 * CALIFORNIA, NOR ANY OF THEIR EMPLOYEES, MAKES ANY WARRANTY, EXPRESS OR 
 * IMPLIED, OR ASSUMES ANY LEGAL LIABILITY OR RESPONSIBILITY FOR THE ACCURACY, 
 * COMPLETENESS, OR USEFULNESS OF ANY INFORMATION, APPARATUS, PRODUCT, OR 
 * PROCESS DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE PRIVATELY 
 * OWNED RIGHTS.

 * Additionally, this program is free software; you can distribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation; either version 2 of the License, 
 * or any later version.  Accordingly, this program is distributed in the hope 
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU Lesser General Public License for more details.
 */
/*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%*/


#ifndef QUADRICS_COLL_H
#define QUADRICS_COLL_H

#include <elan3/elan3.h>

#ifdef DBG_BCAST  /* If debug is enabled. --Weikuan */

#define START_MARK  \
do { printf("VP %4d: ", myproc());             \
  printf("Enter %-10s %s \n", __FUNCTION__, __FILE__); fflush(stdout);} \
while (0)

#define END_MARK                                                        \
do { printf("VP %4d: ", myproc());                                      \
  printf("END %-10s %s %d\n", __FUNCTION__, __FILE__, __LINE__);        \
  fflush(stdout);}                                                      \
while (0)

#define YUW_MARK                                                        \
do { printf("VP %4d: ", myproc());                                      \
  printf("%-10s %d \n", __FUNCTION__, __LINE__); fflush(stdout);}       \
while (0)

#else                     /* If debug is not needed */

#define START_MARK  
#define END_MARK  
#define YUW_MARK

#endif                    /* End of ! DBG_BCAST */

#ifdef USE_ELAN_COLL    
/* The number of the broadcaster supported and The size of the global buffer 
 * used per broadcast bcast in main and elan memory 
 */
#define MAX_BROADCASTERS                 (64)
#define NR_BROADCASTER                   (8)
#define BCAST_CTRL_SIZE                  (4096)
#define COMM_BCAST_MEM_SIZE              (2*1024*1024 + 128*16)
#define COMM_BCAST_ELAN_SIZE             (64 * 4 * 32) 

/* Global main memory of 16M, supporting 8 pools of 2M each */
#define QUADRICS_GLOB_MEM_MAIN    \
  (NR_BROADCASTER * (COMM_BCAST_MEM_SIZE + BCAST_CTRL_SIZE))

/* Global Elan memory for collective dma structures */
#define QUADRICS_GLOB_MEM_ELAN    \
    (NR_BROADCASTER * COMM_BCAST_ELAN_SIZE)

/* Number of channels per broadcaster */
#define NR_CHANNELS                      (16)
#define	BCAST_CHAN_LENGTH                (128 * 1024 + 128)

/* Define a type for virtual memory address in the main memory */
typedef void     * maddr_vm_t ;

#ifndef ELAN_ADDR_NULL
#define ELAN_ADDR_NULL 0
#endif

#ifndef ELAN_ALIGNUP
#define ELAN_ALIGNUP(x,a)       (((uintptr_t)(x) + ((a)-1)) & (-(a)))
                                /* 'a' power of 2 */
#endif

#define M2E(M)  (E3_Addr) elan3_main2elan(ctx, (caddr_t) (M))
#define S2E(S)  (elan3_sdram2elan(ctx, ctx->sdram, (S)))
#define ALLOC_ALIGN                      (64)
                                         
#define SYNC_BRANCH_RATIO                (2)
#define MAX_NO_RAILS                     (2)

/* Protocol cutoff size for messages with different data size */
#define BCAST_INLINE_SIZE_SMALL          (16)
#define BCAST_INLINE_SIZE                (80)
#define BCAST_SMALLMESG                  (16384)
#define MAX_BCAST_FRAGS                  (2)
                                         
/* Timeout value for different errors, not used a lot for now.*/
#define BCAST_RECV_TIMEOUT               (4)
#define BCAST_SEND_TIMEOUT               (4)
#define BCAST_SYNC_TIMEOUT               (8)
#define BCAST_RAIL_TIMEOUT               (16)

/* To find out whether the address is likely to be in the region 
 * related to elan3 hardware bug 2, last 64 bytes of a page.
 */
#define E3_DMA_REVB_BUG_2(SIZE, ADDR, PAGESIZE)           \
( (((int) (ADDR) & (PAGESIZE-64)) == (PAGESIZE-64)) && (-(((int) (ADDR) | ~(PAGESIZE-1))) < (SIZE)) )

/* To find out whether ELAN3 have a corresponding virutal address
 * for this piece of memory, from beginnign to end. 
 */
#define ELAN3_ADDRESSABLE(ctx, addr, size)       \
  ((elan3_main2elan(ctx, (char*)addr) != ELAN_BAD_ADDR) &&      \
  (elan3_main2elan(ctx, (char*)addr + size - 1) != ELAN_BAD_ADDR))

/* 
 * To mark different role of the channels.
 */
enum 
{ 
  BCAST_IDLE_CHANNEL=0,              /* Channel is idle */
  BCAST_SEND_CHANNEL=1,              /* A send channel  */
  BCAST_BUFF_CHANNEL=2,              /* A send channel with buffered data*/
  BCAST_RECV_CHANNEL=3,              /* A recv channel  */
  BCAST_USMP_CHANNEL=4,              /* A channel for a process on SMP */
  BCAST_COMP_CHANNEL=5,              /* A channel just done using */
  BCAST_CHANNEL_TYPES
};

/* 
 * To mark the type of the message for different processing
 */
enum
{
  BCAST_MESG_TYPE_INVALID  = 0,
  BCAST_MESG_TYPE_INLINE_S = 1,
  BCAST_MESG_TYPE_INLINE_L = 2,
  BCAST_MESG_TYPE_SHORT    = 3,
  BCAST_MESG_TYPE_LONG     = 4,
  BCAST_MESG_TYPE_BUGGY    = 5,
  BCAST_MESG_TYPE_SYNC     = 6,
  BCAST_MESG_TYPE_NACK     = 7,
  BCAST_NUM_COLL_MESG_TYPE
};

/* This is really an imagination. Nothing has been done to support
 * hardware bcast based collectives over other interconnects
 */
enum {
  NONE_COLL       = 0x0000,
  QSNET_COLL      = 0x0001,
  IBA_COLL        = 0x0002,
  BLUEGENE_COLL   = 0x0004, 
  MAX_COLL        = 3
};

/* 
 * The envelope of this collective request,
 * This should contain enough information about this operation
 * This also must be <= 56 bytes, so that it can be 
 * contained in a block transfer.
 *
 * When the receiver detects the arrival of the message, it also
 * knows what the checksum is. The checksum is 32 bits long.
 *
 */

typedef struct ulm_coll_env
{
  int                  coll_type;
  int                  comm_index;
  int                  group_index;
  int                  desc_index;
  /*  4 * sizeof (int) */

  int                  root;
  int                  tag_m;	
  int                  key;	
  int                  checksum;		
  /*  4 * sizeof (int) */
} 
ulm_coll_env_t;

#define      HEADER_COMMON                                                 \
  int                  coll_type;                                          \
  int                  comm_index;                                         \
  int                  group_index;                                        \
  int                  desc_index;                                         \
  /* 4*sizeof(int) */                                                      \
                                                                           \
  int                  root;                                               \
  int                  tag_m;	                                           \
  /* 2*sizeof(int) */                                                      \
                                                                           \
  int                  key;                                                \
  int                  mesg_length;   /* The length of contiguous data*/   \
  int                  frag_length;   /* The length of the frag       */   \
  int                  data_length    /* The length of the dma        */   \
  /* 4*sizeof(int) */                                                      


typedef struct quadrics_short_header
{
  HEADER_COMMON;

  /* 16 bytes */
  unsigned char        inline_data[16];  

  int                  data_checksum;  /* crc/checksum for the data    */
  int                  checksum;       /* the checksum for the header  */		
  int                  data[0];        /* The start of real data */
} 
short_header_t;

typedef struct quadrics_long_header
{
  HEADER_COMMON;
  /* 16 bytes */
  unsigned char        inline_data[BCAST_INLINE_SIZE];  

  int                  data_checksum;  /* crc/checksum for the data    */
  int                  checksum;       /* the checksum for the header  */		
  int                  data[0];        /* The start of real data */
} 
long_header_t;

typedef union quadrics_coll_header
{
  /* Many fields are not used, reserve them for later optimization */
  short_header_t      sform;
  long_header_t       lform;
}
quadrics_coll_header_t;

/* 
 * Control structure for a collective operation, 
 * These does not have to be in global elan addressable memory
 */
typedef struct quadrics_channel
{
    /* link back to the actual request */
    bcast_request_t   *ulm_request; 

    /* 
     * Since there are no collective ulm_request,
     * Some information needs to be recorded here
     */
    int                comm;

    /* Could be free, inited, started, acked, completed. --Weikuan */ 
    int                status;
    unsigned int       tag;
    int                toack;          /* acknowledgement needed ?*/
    int                crc;            /* checksum needed        ?*/

    ULMType_t         *data_type;	
    int                comm_type;       /* type of the collective */

    int                root;           /* The root process */

    int                count;
    int	               seqno;
    int	               index;          /* index in the queue */
    int	               send_mode;

#if ENABLE_RELIABILITY
    /* Retransmission Stuff */
    double             time_started;   
    double             time_resend;   
    int                repeats;        /* Number of repeats */
#endif

    /* memory locations */
    maddr_vm_t         appl_addr; 
    maddr_vm_t         mcast_buff;
    maddr_vm_t         offset;          /* where the real data starts  */
}
quadrics_channel_t;

typedef struct quadricsGlobInfo
{
    /* The elan context for the collectives */
    ELAN3_CTX         *ctx;
    ELAN3_CTX         **allCtxs;

    /* The elan global info for the process */
    int                dmaType;
    int                waitType;
    int                retryCount;

    int                nrails;
    int                nvp;
    int                self;   /* elan vp */

    int                nhosts;
    int                hostid;

    int                nlocals;
    int                maxlocals;
    int                localId;
    int                globalId;

    ELAN_LOCATION      myloc;
    ELAN_CAPABILITY   *cap;    /* keep a pointer to the global cap*/

    /* The initially allocate global memory */
    maddr_vm_t         globMainMem;

    /* The initially allocate global elan memory, one per rail */
    sdramaddr_t        globElanMem;
    int                globMainMem_len;
    int                globElanMem_len;
} quadricsGlobInfo_t;

/* The following is just a scaffold for now, not used as yet */
typedef struct quadrics_global_vm_pool
{
  int                  pool_index;
                      
  /* the number of communicators referring this pool */
  int                  ref_count; 
                       
  void            *    base_main;
  void            *    base_elan;
                       
  void            *    start_main;
  void            *    start_elan;
  void            *    current_main;
  void            *    current_elan;
                       
  int                  length_main;
  int                  length_elan;
  int                  chunk_size_main;
  int                  chunk_size_elan;
  int                  avail_main;
  int                  avail_elan;
  int                  num_chunks ; 
  int                  padding; 
} quadrics_global_vm_pool_t;

typedef struct coll_sync
{
  /* synchronization structures */
  sdramaddr_t          glob_event[NR_CHANNELS]; /* 16*8 */
  sdramaddr_t          nack_event[NR_CHANNELS]; 

  /* subtree control structures */
  E3_Event_Blk        *recv_blk_main[NR_CHANNELS];
  E3_Event_Blk        *nack_blk_main[NR_CHANNELS];

  E3_Event_Blk        *glob_env;            /*env Blk, 64 bytes */

  /* A set of structure for src events */
  E3_Event_Blk        *src_blk_main;
  sdramaddr_t          src_blk_elan;
  sdramaddr_t          src_event; /* 16*8 */
  sdramaddr_t          src_wait_ack;
  sdramaddr_t          src_wait_nack;
  /* 16*8 */
                       
  /*sdramaddr_t          wait_e[NR_CHANNELS];*/
                       
  E3_DMA_MAIN         *sync_dma;      /* root uses it for bcast */
  sdramaddr_t          sync_dma_elan; /* corresponding elan structure */
  sdramaddr_t          chain_dma_elan; /* chained dma for ack */
  sdramaddr_t          chain_dma_elan_nack; /* chained dma for nack */
}
coll_sync_t;

typedef struct bcast_ctrl
{
  /* sets of notification structures */
  E3_Event_Blk        *src_blk_main; 
  sdramaddr_t          src_blk_elan;
                       
  E3_Event_Blk        *recv_blk_main[NR_CHANNELS]; 
  sdramaddr_t          glob_event[NR_CHANNELS]; /* 16*8 */
}
bcast_ctrl_t;

typedef struct segment {
    struct segment *next;            /* link between segments            */
    int             bcastVp;         /* The bcast vp                     */
    int             ctx;             /* The context from ELAN_LOCATION   */

    int             min;             /* The min node from ELAN_LOCATION  */
    int             max;             /* The max node from ELAN_LOCATION  */
    int             minVp;           /* The minVp                        */
    int             maxVp;           /* The maxVp                        */

    sdramaddr_t     dma_elan;        /* E3_DMA for data              */
    sdramaddr_t     event;           /* one E3_Event                 */
} bcast_segment_t;

class Broadcaster
{
public:
    int                id;             /* The id of this bcaster */
    int                inuse;          /* Is it in use? */

    maddr_vm_t         base;           /* The base of the global memory */
    maddr_vm_t         top;            /* The top of the global memory */
    int                page_size;      /* The page size used by ELAN3 */

/* Dynamic ones:*/                           
    int                comm_index;     /* hook to the communicator */
    int                queue_index;    /* seqno for the next index to queue */
    int                ack_index;      /* seqno for the next index to ack */
                                       
    int                desc_avail;     /* Number of available channels    */
    int                total_channels; /* Number of channels    */
    int                faulted;        /* a fault occured before next sync?*/
                                       
    int                groupSize;      /* The size of the group */
    int                groupRoot;      /* The root of the group */
    int                localRoot;      /* The local master      */
    int                self;           /* The self id in the group */
    int                nhosts;         /* number of hosts in group */
    int                host_index;     /* The host index in group */

    int                master_vp;      /* The manager process */
    int                parent_vp;      /* The parenet process */
    int                num_branches;   /* The number of branches */
    int                local_master_vp;   /* The local_master vp */
    int                self_vp;        /* The self vp */
    int                local_size;     /* The number of vps */
    int                branchIndex;    /* The branch Index to the parent*/
    int                branchRatio;    /* the branchRation of the tree */
    bcast_segment_t   *segment;        /* The list of segments  */

/* Static Ones */

    /* Sync and ctrl structure for maximally 16 outstanding bcast */
    coll_sync_t       *sync;           /* The synchronization structures */
    bcast_ctrl_t      *ctrl;           /* The transmission structures */

    quadrics_channel_t ** channels;    /* The list of channels */

    // constructor and destructor
    Broadcaster()      {}
    ~Broadcaster()     {}

    /* 
     * Function to enable the use of hardware based collective 
     * This function is to be called when creating new communicators
     * Also this will be invoked by  lampi_init_postfork_coll_setup 
     * to enable the collective support ULM_COMM_WORLD and ULM_COMM_SELF,
     * which was not done in lampi_init_postfork_resources()
     */

    /* The initianization functions */
    int                init_bcaster(maddr_vm_t, sdramaddr_t);
    void               segment_create(void);
    void               init_segment_dma(bcast_segment_t *temp);
    int 	       create_syncup_tree();
    int                reset_coll_events();
    int                init_coll_events(int, int, int);
    int                hardware_coll_init(void);

    /* The reset function, not really free the broadcaster, 
     * but reset it for any other communicator */
    int                broadcaster_free( );
                       
    /* The messaging functions */
    int                bcast_send (quadrics_channel_t * channel);
    int                bcast_recv (quadrics_channel_t * channel);

    /* The progress engine */
    int                make_progress_bcast();

    /* The synchronization functions */
    void               update_channels(int next_toack);
    int                check_channels();
    int 	       sync_parent(int errorcode);
    int 	       sync_leaves(int errorcode);
    int 	       resync_bcast();
};

#endif                             /* End of USE_ELAN_COLL   */

#endif                             /* End of QUADRICS_COLL_H */
