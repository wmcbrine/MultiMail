/*****************************************************************************/
/*                                                                           */
/*           The Blue Wave Offline Mail System Packet Structures             */
/*   Copyright 1990-1995 by Cutting Edge Computing.  All rights reserved.    */
/*                        Created by George Hatchew                          */
/*                                                                           */
/*                      Version 3 - November 30, 1995                        */
/*                                                                           */
/*        ---------------------------------------------------------          */
/*            DISTRIBUTION OF THIS FILE IS LIMITED BY THE TERMS              */
/*           SPECIFIED IN THE BLUE WAVE STRUCTURE DOCUMENTATION!             */
/*        ---------------------------------------------------------          */
/*                                                                           */
/*     These data structures should be usable with any C compiler that       */
/*      supports the 1989 ANSI/ISO C language standard.  They are NOT        */
/*    guaranteed to be usable with older compilers, which largely relied     */
/*  on the definition of the language as specified in Kernighan & Ritchie's  */
/*               _The C Programming Language (1st Edition)_.                 */
/*                                                                           */
/*****************************************************************************/

#ifndef __BLUEWAVE_H    /*  An extra safeguard to prevent this header from  */
#define __BLUEWAVE_H    /*  being included twice in the same source file    */


#define PACKET_LEVEL    3       /* The current mail packet revision level, */
                                /*   used in the "ver" field of the *.INF  */
                                /*   file header.                          */


/*
**  This header defines the data structures for the following files in the
**  official Blue Wave offline mail specification:
**
**      Door:       *.INF       BBS and message area information
**                  *.MIX       Quick index to *.FTI records
**                  *.FTI       Information for all packet messages
**                  *.DAT       Packet message text
**
**      Reader:     *.NET       NetMail reply message information
**                  *.UPI       Information for all other reply messages
**                  *.UPL       Reply message information
**                                (alternative to *.NET and *.UPI)
**                  *.REQ       List of files to download from BBS
**                  *.PDQ       Offline door configuration information
**                                (packet version 2 and earlier)
**                  *.OLC       Offline door configuration information
**                                (packet version 3 and later)
**
**      Misc:       *.MSG       Fido-style message header
**                                  (used *only* in the *.NET structure)
**                  *.XTI       Extended message packet information
**                                  (not an official part of the Blue Wave
**                                  packet specification; is used by the Blue
**                                  Wave reader only)
**
**  The door files (plus individual files for BBS bulletins) comprise a Blue
**  Wave message packet, and the reader files (plus individual files for each
**  message) comprise a Blue Wave reply packet.
**
**  In order to cover ALL BASES, and to be able to say that you were warned,
**  *ALL* unused fields should be set to ASCII NUL (0).  Any future
**  implementation of reserved fields will rely on the premise that the field
**  will be 0 if not implemented!  The same warning follows for BITMAPPED
**  fields.  If a bit is not implemented or is not used, TURN IT OFF (0).
**  (Clearing an entire structure can be easily accomplished via the memset()
**  function.  Example: "memset(&ftirec, 0, sizeof(FTI_REC))".)
*/


/*****************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>>>  DATA TYPE DEFINITIONS  <<<<<<<<<<<<<<<<<<<<<<<<< */
/*****************************************************************************/


/*
**  The data type definitions below help make these structures a little more
**  universal between environments.  The 8-bit, 16-bit, and 32-bit data types
**  defined below can be used as-is with virtually all MS-DOS and OS/2 C/C++
**  compilers, but can be changed if necessary should your compiler define
**  data types in a different fashion.  (Note that the tCHAR and tINT types
**  are currently not used; they are included simply for completeness.)
**
**  If you are programming for a system that employs a CPU which stores multi-
**  byte integers in a manner other than in Intel format (LSB-MSB, or "little
**  endian"), simply #define BIG_ENDIAN before #including this header.  As
**  shown below, this will define the data types as arrays of bytes; the
**  drawback is that *YOU* will have to write functions to convert the data,
**  since the Blue Wave packet specification requires the data to be in Intel-
**  style little-endian format.
**
**  IMPORTANT NOTE ABOUT COMPILERS AND STRUCTURES:
**  All structures *must* be "packed" (i.e., the compiler MUST NOT insert
**  padding bytes between structure elements in order to force elements onto
**  word boundaries).  The Blue Wave products expect them to be packed; if
**  they aren't, you're bound to get some *very* interesting results.
*/

#ifdef BIG_ENDIAN

typedef signed char    tCHAR;     /* 8 bit signed values           */
typedef unsigned char  tBYTE;     /* 8 bit unsigned values         */
typedef unsigned char  tINT[2];   /* little-endian 16 bit signed   */
typedef unsigned char  tWORD[2];  /* little-endian 16 bit unsigned */
typedef unsigned char  tLONG[4];  /* little-endian 32 bit signed   */
typedef unsigned char  tDWORD[4]; /* little-endian 32 bit unsigned */

#else

typedef signed char    tCHAR;     /* 8 bit signed values    */
typedef unsigned char  tBYTE;     /* 8 bit unsigned values  */
typedef signed short   tINT;      /* 16 bit signed values   */
typedef unsigned short tWORD;     /* 16 bit unsigned values */
typedef signed long    tLONG;     /* 32 bit signed values   */
typedef unsigned long  tDWORD;    /* 32 bit unsigned values */

#endif


/*****************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>>  DOOR DATA FILE STRUCTURES  <<<<<<<<<<<<<<<<<<<<<<< */
/*****************************************************************************/


/*
**  Name of file:   *.INF
**
**  Description:    The *.INF file is the source of information for just about
**                  everything from the host BBS, as well as definitions for
**                  all of the message areas that are available to the user
**                  and their status (Local, EchoMail, NetMail, Read Only,
**                  etc.).
**
**  File format:    INF_HEADER          { only included one time!        }
**                  INF_AREA_INFO       { repeated for as many msg bases }
**                  INF_AREA_INFO       { as are available to the user   }
**                  ...
*/

/*  Bit-masks for INF_HEADER.UFLAGS field  */

#define INF_HOTKEYS     0x0001      /* User uses "hotkeys" in door prompts   */
#define INF_XPERT       0x0002      /* Short menus displayed in door         */
#define INF_RES1        0x0004      /* RESERVED -- DO NOT USE!               */
#define INF_GRAPHICS    0x0008      /* Enable ANSI control sequences in door */
#define INF_NOT_MY_MAIL 0x0010      /* Do not bundle mail from user          */
#define INF_EXT_INFO    0x0020      /* Download extended info with messages  */
                                    /*   (* VERSION 3 AND LATER ONLY *)      */
#define INF_NUMERIC_EXT 0x0040      /* Use numeric extensions on packets     */
                                    /*   (* VERSION 3 AND LATER ONLY *)      */

/*  Bit-masks for INF_HEADER.NETMAIL_FLAGS field  */

#define INF_CAN_CRASH   0x0002      /* Allow Crash status          */
#define INF_CAN_ATTACH  0x0010      /* Allow File Attach messages  */
#define INF_CAN_KSENT   0x0080      /* Allow Kill/Sent status      */
#define INF_CAN_HOLD    0x0200      /* Allow Hold status           */
#define INF_CAN_IMM     0x0400      /* Allow Immediate status      */
#define INF_CAN_FREQ    0x0800      /* Allow File Request messages */
#define INF_CAN_DIRECT  0x1000      /* Allow Direct status         */

/*  Bit-masks for INF_HEADER.CTRL_FLAGS field  */

#define INF_NO_CONFIG   0x0001      /* Do not allow offline configuration */
#define INF_NO_FREQ     0x0002      /* Do not allow file requesting       */

/*  Values for INF_HEADER.FILE_LIST_TYPE field  */

#define INF_FLIST_NONE      0       /* Door does not generate a list file  */
#define INF_FLIST_TEXT      1       /* Door generates plain text list file */
#define INF_FLIST_ANSI      2       /* Door generates ANSI list file       */

typedef struct      /*  INF_HEADER  */
{
    tBYTE ver;                  /* Packet version type (currently 2)        */
    tBYTE readerfiles[5][13];   /* Files to be displayed by reader          */
    tBYTE regnum[9];            /* User's registration number               */
    tBYTE mashtype;             /* Currently unused (door fills with 0)     */
                                /*   Reserved for Blue Wave reader to store */
                                /*   the compression type the packet uses.  */
    tBYTE loginname[43];        /* Name user types at BBS login             */
    tBYTE aliasname[43];        /* User's "other" name                      */
    tBYTE password[21];         /* Password                                 */
                                /*   All bytes should be the actually ASCII */
                                /*   value plus 10.  Lame security, yes,    */
                                /*   but it does prevent "TYPE *.INF" from  */
                                /*   showing the password.                  */
    tBYTE passtype;             /* Password type                            */
                                /*   0=none 1=door 2=reader 3=both          */
    tWORD zone;                 /* Main network address of host BBS         */
    tWORD net;                  /*   (zone:net/node.point)                  */
    tWORD node;
    tWORD point;
    tBYTE sysop[41];            /* Name of SysOp of host BBS                */
    tWORD ctrl_flags;           /* Flags to control reader capabilities     */
                                /*   (* VERSION 3 AND LATER ONLY *)         */
    tBYTE systemname[65];       /* Name of host BBS                         */
    tBYTE maxfreqs;             /* Max number of file requests allowed      */
    tWORD is_QWK;               /* Whether *.INF belongs to a QWK packet    */
    tBYTE obsolete2[4];         /* OBSOLETE -- DO NOT USE!                  */
    tWORD uflags;               /* Bit-mapped door options/toggles          */
    tBYTE keywords[10][21];     /* User's entire set of door keywords       */
    tBYTE filters[10][21];      /* User's entire set of door filters        */
    tBYTE macros[3][80];        /* User's door bundling command macros      */
    tWORD netmail_flags;        /* Bit-mapped NetMail options               */
    tWORD credits;              /* NetMail credits                          */
    tWORD debits;               /* NetMail debits                           */
    tBYTE can_forward;          /* 0=Message forwarding not allowed         */
    tWORD inf_header_len;       /* Size of INF_HEADER structure             */
    tWORD inf_areainfo_len;     /* Size of INF_AREA_INFO structure          */
    tWORD mix_structlen;        /* Size of MIX_REC structure                */
    tWORD fti_structlen;        /* Size of FTI_REC structure                */
    tBYTE uses_upl_file;        /* If this field is not zero, the door that */
                                /*   created this packet can receive reply  */
                                /*   packets in the new *.UPL file format.  */
                                /*   Otherwise, the old *.UPI and *.NET     */
                                /*   files must be used.                    */
    tBYTE from_to_len;          /* The maximum length of the FROM: and TO:  */
                                /*   fields that the host BBS can support.  */
                                /*   If this value is 0 or is greater than  */
                                /*   35, then 35 must be used (the upload   */
                                /*   file formats only allow for a maximum  */
                                /*   of 35 characters).                     */
    tBYTE subject_len;          /* The maximum length of the SUBJECT: field */
                                /*   that the host BBS can support.  If     */
                                /*   this value is 0 or is greater than 71, */
                                /*   then 71 must be used (the upload file  */
                                /*   formats only allow for a maximum of 71 */
                                /*   characters).                           */
    tBYTE packet_id[9];         /* Original root name of the mail packet,   */
                                /*   as specified by the mail door.  All    */
                                /*   files in the packet that are created   */
                                /*   by the mail door will use this root    */
                                /*   name, as will the reader when creating */
                                /*   the upload files.  Thus, even if the   */
                                /*   packets themselves are renamed to      */
                                /*   something completely different, the    */
                                /*   mail doors and readers will still be   */
                                /*   able to work with the proper files.    */
    tBYTE file_list_type;       /* New file listing type                    */
                                /* (* VERSION 3 AND LATER ONLY *)           */
                                /*   Specifies the type of new file list    */
                                /*   that is generated by the door (see     */
                                /*   INF_FLIST_xxx, above).  This field is  */
                                /*   intended for use with offline config.  */
    tBYTE auto_macro[3];        /* Auto-macro indicator flags               */
                                /* (* VERSION 3 AND LATER ONLY *)           */
                                /*   Specifies which macros are auto macros */
                                /*   (i.e. execute automatically after mail */
                                /*   is scanned).                           */
    tINT max_packet_size;       /* Maximum size of uncompressed packet      */
                                /* (* VERSION 3 AND LATER ONLY *)           */
                                /*   Specifies, in K, the maximum size of   */
                                /*   an uncompressed mail packet.  A value  */
                                /*   of 0 indicates no maximum length.      */
                                /*   This field is intended for use with    */
                                /*   offline config.                        */
    tBYTE reserved[228];        /* RESERVED FOR FUTURE USE                  */
                                /*   This field MUST be filled with ASCII   */
                                /*   NUL (0x00) characters in order for     */
                                /*   future additional features to work     */
                                /*   properly!                              */
}
INF_HEADER;

/*
**  Notes about the INF_HEADER.XXXXX_LEN fields, above:
**
**  Door authors should take the few extra lines of code to fill in the
**  structure lengths defined above.  Doing so will make the Blue Wave data
**  structures extensible and adaptable to almost any kind of file change that
**  may be required in the future.  The readers that use this mail packet
**  format should contain code to handle a structure length that is longer or
**  shorter than they expect.
**
**  Reader authors need to take the time to code for possible extensions to
**  this file format.  If the data fields are LONGER than expected, simply do
**  a seek to move to the next record, and ignore the extra information.  If
**  the data fields are SHORTER than expected, a simple "Please upgrade your
**  reader" should suffice. <grin>  However, you should never encounter a
**  record size smaller than the ones defined here.  Any extra information
**  that is sent in the packets probably would not be crucial, and you may be
**  able to continue with reading the packet anyway.
**
**  It should be noted that all current Blue Wave doors set these fields to 0,
**  as this extensibility was not added until recently.  If the structure
**  sizes are 0, the reader will assume that all records are of the sizes
**  defined here.  (Blue Wave readers below version 2.10 do NOT allow for
**  extensible data files.  Version 2.10, and all subsequent versions, WILL
**  handle them properly.)  DO NOT EXTEND THESE STRUCTURE FORMATS WITHOUT
**  NOTIFYING CUTTING EDGE COMPUTING FIRST!  If the extended information will
**  benefit programs/users, it will be officially added to the packet format.
**
**  The original values for the INF_HEADER.XXXXX_LEN structures are as below,
**  defined as macros which you can use in your programs.  Remember, if the
**  value in INF_HEADER.XXXXX_LEN is 0, you must use these values instead!
*/

#define ORIGINAL_INF_HEADER_LEN     1230    /* Original *.INF header len   */
#define ORIGINAL_INF_AREA_LEN       80      /* Original *.INF area rec len */
#define ORIGINAL_MIX_STRUCT_LEN     14      /* Original *.MIX record len   */
#define ORIGINAL_FTI_STRUCT_LEN     186     /* Original *.FTI record len   */

/*
**  Below is some sample C code for reading in the variable length *.INF
**  structure, which is the most "difficult" one to do.  Note the sections of
**  code which use the ORIGINAL_XXXXX_LEN macros; these are the sections that
**  determine the proper structure length.  (Comments are preceeded by "#"
**  signs, since using C comment symbols would make most compilers think that
**  nested comments are in use, a practice which normally is not allowed.)
**
**  int read_inf_file(void)
**  {
**      INF_HEADER    inf_header;
**      INF_AREA_INFO inf_info;
**      FILE          *inf_file=NULL;
**      tWORD         record_num=0u;
**      tWORD         inf_header_slen, inf_area_slen;
**      tLONG         seek_pos=0L;
**
**      inf_file = fopen("WILDBLUE.INF", "rb");
**      if (inf_file == NULL)
**          return 0;
**
**      fread(&inf_header, sizeof(INF_HEADER), 1, inf_file);
**      puts(inf_header.loginname);
**      puts(inf_header.aliasname);
**
**      # Test and verify the validity of the structure lengths.
**
**      if (inf_header.inf_header_len < ORIGINAL_INF_HEADER_LEN)
**          inf_header_slen = ORIGINAL_INF_HEADER_LEN;
**      else
**          inf_header_slen = inf_header.inf_header_len;
**
**      if (inf_header.inf_areainfo_len < ORIGINAL_INF_AREA_LEN)
**          inf_area_slen = ORIGINAL_INF_AREA_LEN;
**      else
**          inf_area_slen = inf_header.inf_areainfo_len;
**
**      # now, move to the END of the header, since it may be longer
**      # than we expect it to be.  Use fseek()...
**
**      fseek(inf_file, (long)inf_header_slen, SEEK_SET);
**
**      record_num = 0U;
**      while(fread(&inf_info, sizeof(INF_AREA_INFO), 1, inf_file))
**      {
**          puts(inf_info.title);
**          record_num++;
**
**          # we need to seek past the header, and then [record_num]
**          # number of recs.
**
**          seek_pos = (long)(inf_header_slen+(record_num*inf_area_slen));
**          fseek(inf_file, seek_pos, SEEK_SET);
**      }
**
**      fclose(inf_file);
**      return 1;
**  }
*/

/*  Bit-masks for INF_AREA_INFO.AREA_FLAGS field  */

#define INF_SCANNING    0x0001  /* On=User is active for area               */
#define INF_ALIAS_NAME  0x0002  /* On=Alias name, Off=Login name            */
                                /*   If ON, use INF_HEADER.ALIASNAME when   */
                                /*   addressing new mail or replies for the */
                                /*   message area.  If OFF, the reader uses */
                                /*   the INF_HEADER.LOGINNAME for this      */
                                /*   purpose.                               */
#define INF_ANY_NAME    0x0004  /* On=Allow any name to be entered          */
                                /*   If ON, any name can be entered in the  */
                                /*   From: field when addressing new mail   */
                                /*   or replies for the message area.  If   */
                                /*   OFF, the normal rules apply.           */
#define INF_ECHO        0x0008  /* On=Network mail, Off=Local mail          */
#define INF_NETMAIL     0x0010  /* On=E-mail, Off=Conference mail           */
                                /*   Refer to the chart below (the values   */
                                /*   for the NETWORK_TYPE field) for info   */
                                /*   on how these two flags should be set   */
                                /*   for message areas.                     */
#define INF_POST        0x0020  /* On=User can post, Off=User CANNOT post   */
#define INF_NO_PRIVATE  0x0040  /* On=Private messages are NOT allowed      */
#define INF_NO_PUBLIC   0x0080  /* On=Public messages are NOT allowed       */
#define INF_NO_TAGLINE  0x0100  /* On=Taglines are not allowed              */
#define INF_NO_HIGHBIT  0x0200  /* On=ASCII 1-127 only, Off=ASCII 1-255     */
                                /*   If ON, only ASCII values 1 to 127 are  */
                                /*   allowed in messages.  If OFF, all      */
                                /*   values from 1 to 255 are allowed.  Due */
                                /*   to the fact that ASCII value 0 is used */
                                /*   in C as a string terminator, the value */
                                /*   0 should not be allowed in messages at */
                                /*   all.                                   */
#define INF_NOECHO      0x0400  /* On=User can prevent messages from being  */
                                /*   sent through the network               */
#define INF_HASFILE     0x0800  /* On=User can attach files to messages     */
#define INF_PERSONAL    0x1000  /* On=User is downloading only personal     */
                                /*   msgs in this message area.  The flag   */
                                /*   INF_SCANNING also needs to be ON.      */
                                /*   (* VERSION 3 AND LATER ONLY *)         */
#define INF_TO_ALL      0x2000  /* On=User is downloading messages to "All" */
                                /*   and personal messages only in this     */
                                /*   area.  The flag INF_SCANNING also      */
                                /*   needs to be ON.  INF_PERSONAL should   */
                                /*   *not* be set, as this flag implies the */
                                /*   downloading of personal messages also. */
                                /*   (* VERSION 3 AND LATER ONLY *)         */

/*  Values for INF_AREA_INFO.NETWORK_TYPE field  */

#define INF_NET_FIDONET     0   /* FidoNet-style E-mail and conferences     */
                                /*   Local     = INF_ECHO=off, NETMAIL=off  */
                                /*   EchoMail  = INF_ECHO=on,  NETMAIL=off  */
                                /*   GroupMail = INF_ECHO=on,  NETMAIL=off  */
                                /*   NetMail   = INF_ECHO=on,  NETMAIL=on   */
#define INF_NET_INTERNET    1   /* Internet E-mail and Usenet newsgroups    */
                                /*   Local     = INF_ECHO=off, NETMAIL=off  */
                                /*   Newsgroup = INF_ECHO=on,  NETMAIL=off  */
                                /*   E-mail    = INF_ECHO=on,  NETMAIL=on   */

typedef struct      /*  INF_AREA_INFO  */
{
    tBYTE areanum[6];       /* Area number this record corresponds to  */
    tBYTE echotag[21];      /* Area tag name (*.BRD name for Telegard) */
    tBYTE title[50];        /* Area description/title                  */
    tWORD area_flags;       /* Bit-mapped area options                 */
    tBYTE network_type;     /* Network mail type (see above)           */
}
INF_AREA_INFO;

/*---------------------------------------------------------------------------*/

/*
**  Name of file:   *.MIX
**
**  Description:    The *.MIX file is a very small file, with one record for
**                  every message area that was scanned.  It contains the
**                  information to get into the *.FTI file.
**
**  File format:    MIX_REC     { repeated for each message area scanned }
**                  MIX_REC
**                  ...
*/

typedef struct      /*  MIX_REC  */
{
    tBYTE areanum[6];   /* Area number this record corresponds to         */
                        /*   This is the ASCII representation of the      */
                        /*   actual area number shown on the host BBS.    */
    tWORD totmsgs;      /* Total number of messages for this area         */
    tWORD numpers;      /* Total number of personal messages in this area */
    tLONG msghptr;      /* Pointer to first message header in *.FTI file  */
}
MIX_REC;

/*---------------------------------------------------------------------------*/

/*
**  Name of file:   *.FTI
**
**  Description:    The *.FTI file contains the information for each message
**                  in the packet.  Each record includes all of the
**                  information about the message, including the pointer to
**                  the actual message text in the *.DAT file.
**
**                  NOTE:   Messages in the *.FTI file will ALWAYS be in area
**                          number order.  That is to say, if the MIX_REC
**                          indicates there are 100 messages for this area,
**                          all 100 messages will follow in sequential order.
**
**  File format:    FTI_REC     { repeated for as many messages }
**                  FTI_REC     { as obtained from the host BBS }
**                  ...
*/

/*  Bit-masks for FTI_REC.FLAGS field  */

#define FTI_MSGPRIVATE      0x0001  /* Private = For addressee ONLY         */
#define FTI_MSGCRASH        0x0002  /* Crash = High priority mail           */
#define FTI_MSGREAD         0x0004  /* Read = Message read by addressee     */
#define FTI_MSGSENT         0x0008  /* Sent = Message sent                  */
#define FTI_MSGFILE         0x0010  /* File Attach = Send file(s)           */
#define FTI_MSGFWD          0x0020  /* Forward = Message to/from others     */
#define FTI_MSGORPHAN       0x0040  /* Orphan = Message destination unknown */
#define FTI_MSGKILL         0x0080  /* Kill/Sent = Delete after sending     */
#define FTI_MSGLOCAL        0x0100  /* Local = Message originated here      */
#define FTI_MSGHOLD         0x0200  /* Hold = Hold for pickup, don't send   */
#define FTI_MSGIMMEDIATE    0x0400  /* Immediate = Send message NOW         */
#define FTI_MSGFRQ          0x0800  /* File Request = Request file(s)       */
#define FTI_MSGDIRECT       0x1000  /* Direct = Send direct, no routing     */
#define FTI_MSGUNUSED1      0x2000  /*                                      */
#define FTI_MSGUNUSED2      0x4000  /*                                      */
#define FTI_MSGURQ          0x8000  /* Update Request = Req updated file(s) */

typedef struct      /*  FTI_REC  */
{
    tBYTE from[36];         /* Person message is from                       */
    tBYTE to[36];           /* Person message is to                         */
    tBYTE subject[72];      /* Subject/title of message                     */
    tBYTE date[20];         /* Origin date of message                       */
                            /*   Depending on the host BBS's date storage   */
                            /*   format, the EXACT format of this field     */
                            /*   will change.  Some will take all 19 bytes, */
                            /*   others may take only 10.                   */
    tWORD msgnum;           /* Number of THIS message on BBS                */
    tWORD replyto;          /* "This is a reply to #xx"                     */
                            /*   Not used for every message.  When non-     */
                            /*   zero, there is a previous message in       */
                            /*   the thread.                                */
    tWORD replyat;          /* "There is a reply at #xx"                    */
                            /*   Not used for every message.  When non-     */
                            /*   zero, there is a reply to this message.    */
    tLONG msgptr;           /* Offset to start of message in *.DAT file     */
                            /*   Seek to this exact offset in the *.DAT     */
                            /*   file, then read "msglength" bytes from     */
                            /*   the file to load the entire message text.  */
    tLONG msglength;        /* Length of message text (in bytes)            */
    tWORD flags;            /* Bit-mapped message status flags              */
    tWORD orig_zone;        /* Origin address of message                    */
                            /*   These three fields will most likely be 0,  */
                            /*   unless the current message belongs to a    */
                            /*   NetMail message base.                      */
    tWORD orig_net;
    tWORD orig_node;
}
FTI_REC;

/*---------------------------------------------------------------------------*/

/*
**  Name of file:   *.DAT
**
**  Description:    The *.DAT file is an unstructured file which contains the
**                  text of every message obtained from the host BBS.
**                  Valid messages begin with an ASCII space (0x20) character
**                  (which is NOT to be considered part of the message!)
**                  followed by zero or more bytes which constitute the
**                  message text.  The pointer to the text for each message is
**                  stored in FTI_REC.MSGPTR, and the length of the text for
**                  each message is stored in FTI_REC.MSGLENGTH.
**
**  File format:    Unstructured
*/


/*****************************************************************************/
/* >>>>>>>>>>>>>>>>>  MISCELLANEOUS DATA FILE STRUCTURES  <<<<<<<<<<<<<<<<<< */
/*****************************************************************************/


/*
**  Name of file:   *.MSG
**
**  Description:    The Fido *.MSG message (named for the BBS program on which
**                  it originated) has become a de-facto standard among BBS
**                  implementations, due to the sheer number of utilities
**                  available that operate with *.MSG messages.  It is as
**                  close to a universal message format as one can get in
**                  FidoNet (and FidoNet-style networks), and is the reason
**                  why it is used here (well, the *.MSG header, anyway).
**
**                  NOTE:   Most of the fields in the FTI_REC structure (shown
**                          above) correspond to similar fields in MSG_REC.
**                          This was done deliberately, in order to make
**                          *.FTI file processing a little more intuitive for
**                          programmers.  Also note that MSG_REC is only used
**                          by the NET_REC structure, which will soon become
**                          obsolete (replaced by UPL_REC).
**
**  File format:    MSG_REC         { only included one time!                }
**                  message text    { text can be terminated by an ASCII NUL }
**                                  { character (0x00), or by an ASCII CR,   }
**                                  { LF, NUL (0x0D 0x0A 0x00) sequence      }
*/

/*  Bit-masks for MSG_REC.ATTR field  */

#define MSG_NET_PRIVATE     0x0001  /* Private                */
#define MSG_NET_CRASH       0x0002  /* Crash mail             */
#define MSG_NET_RECEIVED    0x0004  /* Received               */
#define MSG_NET_SENT        0x0008  /* Sent                   */
#define MSG_NET_FATTACH     0x0010  /* File attached          */
#define MSG_NET_INTRANSIT   0x0020  /* In-transit             */
#define MSG_NET_ORPHAN      0x0040  /* Orphaned               */
#define MSG_NET_KILL        0x0080  /* Kill after sending     */
#define MSG_NET_LOCAL       0x0100  /* Local message          */
#define MSG_NET_HOLD        0x0200  /* Hold for pickup        */
#define MSG_NET_RESERVED    0x0400  /* RESERVED               */
#define MSG_NET_FREQ        0x0800  /* File request           */
#define MSG_NET_RREQ        0x1000  /* Return receipt request */
#define MSG_NET_RECEIPT     0x2000  /* Return receipt message */
#define MSG_NET_AREQ        0x4000  /* Audit request          */
#define MSG_NET_FUREQ       0x8000  /* File update request    */

typedef struct      /*  MSG_REC (will soon be obsolete)  */
{
    tBYTE from[36];     /* Person message is from                           */
    tBYTE to[36];       /* Person message is to                             */
    tBYTE subj[72];     /* Subject/title of message                         */
    tBYTE date[20];     /* Creation date/time                               */
                        /*   This date/time is usually in either of the     */
                        /*   Fido-sanctioned formats "DD MMM YY  HH:MM:SS"  */
                        /*   or "WWW DD MMM YY HH:MM", but due to the       */
                        /*   chaotic nature of FidoNet-compatible software, */
                        /*   this CANNOT be relied upon!                    */
    tWORD times;        /* Number of times read (fairly obsolete)           */
    tWORD dest;         /* Destination node (of net/node)                   */
    tWORD orig;         /* Origin node (of net/node)                        */
    tWORD cost;         /* Cost of sending message (usually in US cents)    */
    tWORD orig_net;     /* Origin net (of net/node)                         */
    tWORD destnet;      /* Destination net (of net/node)                    */
    tLONG unused1;      /* Undefined                                        */
    tLONG unused2;      /*   Some software (Opus and Maximus, for example)  */
                        /*   uses these fields to store the sent/received   */
                        /*   date/time as bit-packed fields, using the same */
                        /*   format used in MS-DOS directory entries.       */
    tWORD reply;        /* Message # that this message replies to           */
    tWORD attr;         /* Message attributes and behavior flags            */
    tWORD up;           /* Message # that replies to this message           */
}
MSG_REC;

/*---------------------------------------------------------------------------*/

/*
**  Name of file:   *.XTI
**
**  Description:    The *.XTI file contains extended information for each
**                  message in the packet.  The number of records in the *.XTI
**                  file will always equal the number of messages in the
**                  packet, with each record corresponding to a record in the
**                  *.FTI file (i.e. record #1 in the *.XTI file corresponds
**                  to record #1 in the *.FTI file, and so on).
**
**                  NOTE:   This file is currently created ONLY by the Blue
**                          Wave reader, and is not a part of the official
**                          Blue Wave packet specification; it is merely
**                          documented here for third party programmers to use
**                          if they so desire.  How other readers store which
**                          messages have been read/replied-to/marked is left
**                          as an option to be implemented by the individual
**                          reader authors.  You may use this method if you so
**                          desire; however, PLEASE do not name any external
**                          files not conforming to this specification as
**                          <packet-ID>.XTI, due to the fact that the Blue
**                          Wave reader will expect the file to be in the
**                          format described.  If it's not in the expected
**                          format, things will get interesting. :-)
**
**  File format:    XTI_REC     { repeated for as many messages }
**                  XTI_REC     { as obtained from the host BBS }
**                  ...
*/

/*  Bit-masks for XTI_REC.FLAGS field  */

#define XTI_HAS_READ        0x01    /* Message has been read            */
#define XTI_HAS_REPLIED     0x02    /* Message has been replied to      */
#define XTI_IS_PERSONAL     0x04    /* Message is personal              */
#define XTI_IS_TAGGED       0x08    /* Message has been 'tagged'        */
#define XTI_HAS_SAVED       0x10    /* Message has been saved           */
#define XTI_HAS_PRINTED     0x20    /* Message has been printed         */

/*  Bit-masks for XTI_REC.MARKS field  */

#define XTI_MARK_SAVE       0x01    /* Message marked for saving   */
#define XTI_MARK_REPLY      0x02    /* Message marked for replying */
#define XTI_MARK_PRINT      0x04    /* Message marked for printing */
#define XTI_MARK_DELETE     0x08    /* Message marked for deletion */

typedef struct      /*  XTI_REC  */
{
    tBYTE flags;    /* Bit-mapped message flags   */
    tBYTE marks;    /* Bit-mapped message markers */
}
XTI_REC;


/*****************************************************************************/
/* >>>>>>>>>>>>>>>>>>>>  READER DATA FILE STRUCTURES  <<<<<<<<<<<<<<<<<<<<<< */
/*****************************************************************************/


/*
**  Name of file:   *.NET
**
**  Description:    The *.NET file is created ONLY when there is NetMail to be
**                  sent.  It contains the FULL header of the Fido-style *.MSG
**                  structure plus the fields defined below (which aren't part
**                  of the standard *.MSG structure yet required by the door).
**
**                  NOTE:   Readers should only generate a *.NET file if
**                          INF_HEADER.USES_UPL_FILE is not set *AND* the
**                          mail packet format is version 2 or earlier.
**                          Doors should process *.NET files *ONLY* in cases
**                          where a *.UPL file is not present.
**
**  File format:    NET_REC     { repeated for as many NetMail    }
**                  NET_REC     { messages as exist in the packet }
**                  ...
*/

typedef struct      /*  NET_REC  */
{
    MSG_REC msg;            /* The Fido-style *.MSG header                */
    tBYTE fname[13];        /* Filename the message text is in            */
    tBYTE echotag[21];      /* NetMail area tag (*.BRD name for Telegard) */
    tWORD zone;             /* Destination zone (of zone:net/node.point)  */
    tWORD point;            /* Destination point (of zone:net/node.point) */
    tLONG unix_date;        /* Date/time of message                       */
                            /*   This Unix-style date/time value (number  */
                            /*   of seconds since 01/01/70) is converted  */
                            /*   to the date/time storage method used by  */
                            /*   the host BBS.                            */
}
NET_REC;

/*---------------------------------------------------------------------------*/

/*
**  Name of file:   *.UPI
**
**  Description:    The *.UPI file contains the information for each message
**                  in the reply packet, as well as information on the reader
**                  version and registration numbers.  Each record includes
**                  all of the information about the message.
**
**                  NOTE:   Readers should only generate a *.UPI file if
**                          INF_HEADER.USES_UPL_FILE is not set *AND* the
**                          mail packet format is version 2 or earlier.
**                          Doors should process *.UPI files *ONLY* in cases
**                          where a *.UPL file is not present.
**
**  File format:    UPI_HEADER      { only included one time!        }
**                  UPI_REC         { repeated for as many msg bases }
**                  UPI_REC         { as are available to the user   }
**                  ...
*/

typedef struct      /*  UPI_HEADER  */
{
    tBYTE regnum[9];    /* Reader registration number                   */
    tBYTE vernum[13];   /* Reader version number                        */
                        /*   All bytes should be the actually ASCII     */
                        /*   value plus 10.  Lame security, yes, but it */
                        /*   does prevent "TYPE *.UPI" from showing the */
                        /*   version number.                            */
    tBYTE future[33];   /* RESERVED FOR FUTURE USE                      */
#ifdef PAD_SIZES_EVEN
    tBYTE evenpad;      /* If your compiler pads structures out to even */
                        /*   numbered sizes, define PAD_SIZES_EVEN      */
                        /*   before including this header.  When the    */
                        /*   *.UPI file is written, be sure to write    */
                        /*   sizeof(UPI_HEADER) - 1 bytes, otherwise    */
                        /*   your compiler may insert an extra byte not */
                        /*   explicitly specified here.                 */
#endif
}
UPI_HEADER;

/*  Bit-masks for UPI_REC.FLAGS field  */

#define UPI_RES1        0x01    /* RESERVED FOR FUTURE USE                   */
#define UPI_RES2        0x02    /* RESERVED FOR FUTURE USE                   */
#define UPI_RES3        0x04    /* RESERVED FOR FUTURE USE                   */
#define UPI_RES4        0x08    /* RESERVED FOR FUTURE USE                   */
#define UPI_RES5        0x10    /* RESERVED FOR FUTURE USE                   */
#define UPI_RES6        0x20    /* RESERVED FOR FUTURE USE                   */
#define UPI_PRIVATE     0x40    /* Message is PRIVATE                        */
#define UPI_NO_ECHO     0x80    /* Message is NOT to be echoed               */
                                /*   This feature is not yet implemented in  */
                                /*   the Blue Wave reader or doors, as none  */
                                /*   of the currently supported BBS software */
                                /*   has support for this feature.           */

typedef struct      /*  UPI_REC  */
{
    tBYTE from[36];     /* Person message is from                     */
    tBYTE to[36];       /* Person message is to                       */
    tBYTE subj[72];     /* Subject/title of message                   */
    tLONG unix_date;    /* Date/time of message                       */
                        /*   This Unix-style date/time value (number  */
                        /*   of seconds since 01/01/70) is converted  */
                        /*   to the date/time storage method used by  */
                        /*   the host BBS.                            */
    tBYTE fname[13];    /* Filename the message text is in            */
    tBYTE echotag[21];  /* Area tag name (*.BRD name for Telegard)    */
    tBYTE flags;        /* Bit-mapped flags                           */
    tBYTE reedit;       /* INTERNAL USE ONLY!                         */
                        /*   This flag is used internally by the Blue */
                        /*   Wave reader.  Doors should ignore this   */
                        /*   field during reply packet processing.    */
}
UPI_REC;

/*---------------------------------------------------------------------------*/

/*
**  Name of file:   *.UPL
**
**  Description:    The *.UPL file contains the information for each message
**                  in the reply packet, as well as information on the reader
**                  version and registration numbers.  Each record includes
**                  all of the information about the message.
**
**                  NOTE:   Readers should only generate a *.UPL file if
**                          INF_HEADER.USES_UPL_FILE is set *AND/OR* the mail
**                          packet format is version 3 or later.  Doors should
**                          process *.UPL files in all cases where one is
**                          present.
**
**  File format:    UPL_HEADER      { only included one time!       }
**                  UPL_REC         { repeated for as many messages }
**                  UPL_REC         { as are included in the packet }
**                  ...
*/

typedef struct      /*  UPL_HEADER  */
{
    tBYTE regnum[10];       /* Reader registration number (if desired)      */
    tBYTE vernum[20];       /* Reader version number as a string.           */
                            /*   All bytes should be the actually ASCII     */
                            /*   value plus 10.  Lame security, yes, but it */
                            /*   does prevent "TYPE *.UPL" from showing the */
                            /*   version number.                            */
                            /*   Examples:  "2.10a Beta"                    */
                            /*              "2.11"                          */
    tBYTE reader_major;     /* Major version of the reader (number to the   */
                            /*   left of the decimal point)                 */
    tBYTE reader_minor;     /* Minor version of the reader (number to the   */
                            /*   right of the decimal point)                */
    tBYTE reader_name[80];  /* String containing name of the reader, such   */
                            /*   as "The Blue Wave Offline Mail Reader".    */
                            /*   This is provided for door programmers that */
                            /*   wish to display the name of the reader     */
                            /*   that created the reply packet.  (Filling   */
                            /*   it is mandatory but using it is optional.) */
    tWORD upl_header_len;   /* Size of UPL_HEADER structure                 */
    tWORD upl_rec_len;      /* Size of UPL_REC structure                    */
                            /*   NOTE:  Refer to the INF_HEADER section for */
                            /*          more information on using the size  */
                            /*          fields.                             */
    tBYTE loginname[44];    /* Name found in INF_HEADER.LOGINNAME.  This is */
                            /*   provided for door authors as a security    */
                            /*   measure to implement as they wish.         */
    tBYTE aliasname[44];    /* Name found in INF_HEADER.ALIASNAME           */
    tBYTE reader_tear[16];  /* String containing abbreviated name of the    */
                            /*   reader, such as "Blue Wave", "Q-Blue",     */
                            /*   "Wave Rider", etc.  This is provided for   */
                            /*   doors programmers that wish to add to the  */
                            /*   tear line the name of the reader that      */
                            /*   created the reply packet.  (Filling it is  */
                            /*   mandatory but using it is optional.)  If   */
                            /*   this field is blank, the tear line to be   */
                            /*   generated is left to the discretion of the */
                            /*   door author.                               */
    tBYTE compress_type;    /* Compression type required for mail packet    */
                            /*   The Blue Wave reader uses this internally  */
                            /*   to store the compression type required for */
                            /*   this particular mail packet.               */
    tBYTE flags;            /* Reader processing flags                      */
                            /*   The Blue Wave reader uses this internally  */
                            /*   to store flags required for later          */
                            /*   processing.                                */
                            /*     0x01 = Was a .QWK packet.                */
                            /*     0x02 = Host requires a *.UPI file        */
    tBYTE not_registered;   /* Reader is not registered to user             */
                            /*   If this byte is set to a non-zero value,   */
                            /*   the Blue Wave doors will assume that the   */
                            /*   user's reader was not registered, and will */
                            /*   place "[NR]" at the end of the tear line.  */
                            /*   Third-party doors may use this flag for    */
                            /*   the same purpose; its use is optional by   */
                            /*   mail readers (especially if you don't care */
                            /*   whether or not "[NR]" shows up on the tear */
                            /*   line <grin>).                              */
    tBYTE pad[33];          /* RESERVED FOR FUTURE USE, and to pad struct   */
                            /*   out to a 'nice' 256 bytes                  */
}
UPL_HEADER;

/*  Bit-masks for UPL_REC.MSG_ATTR field  */

#define UPL_INACTIVE    0x0001  /* Message is INACTIVE                       */
                                /*   Doors should NOT attempt to import this */
                                /*   message.                                */
#define UPL_PRIVATE     0x0002  /* Message is PRIVATE                        */
#define UPL_NO_ECHO     0x0004  /* Message is NOT to be echoed               */
                                /*   This feature is not yet implemented in  */
                                /*   the Blue Wave reader or doors, as none  */
                                /*   of the currently supported BBS software */
                                /*   has support for this feature.           */
#define UPL_HAS_FILE    0x0008  /* Message has file "attached" to it         */
                                /*   It is up to the door to check the       */
                                /*   validity of this flag.  If the file is  */
                                /*   contained in the mail packet, great.    */
                                /*   If not, the door should probably prompt */
                                /*   the user to begin uploading the file    */
                                /*   after importing the messages.  (Not yet */
                                /*   implemented in the Blue Wave reader.)   */
#define UPL_NETMAIL     0x0010  /* Message is network mail                   */
                                /*   Indicates NetMail/E-mail message.  The  */
                                /*   NETWORK_TYPE field (see below) will     */
                                /*   indicate which fields should be used    */
                                /*   for addressing the message.             */
#define UPL_IS_REPLY    0x0020  /* Indicates that the message is a reply to  */
                                /*   an existing message, rather than being  */
                                /*   a completely new message.               */
#define UPL_MRES7       0x0040  /* RESERVED FOR FUTURE USE                   */
#define UPL_MRES8       0x0080  /* RESERVED FOR FUTURE USE                   */
                                /* All of the other 8 bits of this field are */
                                /*   also reserved for future use.  This     */
                                /*   should provide for plenty of expansion  */
                                /*   for future development.                 */

/*  Bit-masks for UPL_REC.NETMAIL_ATTR field  */

#define UPL_NRES1           0x0001  /* RESERVED FOR FUTURE USE             */
#define UPL_NETCRASH        0x0002  /* Crash = High priority mail          */
#define UPL_NRES2           0x0004  /* RESERVED FOR FUTURE USE             */
#define UPL_NRES3           0x0008  /* RESERVED FOR FUTURE USE             */
#define UPL_NETFILE         0x0010  /* File Attach = Send file(s) listed   */
                                    /*   in Subject field                  */
#define UPL_NRES4           0x0020  /* RESERVED FOR FUTURE USE             */
#define UPL_NRES5           0x0040  /* RESERVED FOR FUTURE USE             */
#define UPL_NETKILL         0x0080  /* Kill/Sent = Delete after sending    */
#define UPL_NETLOCAL        0x0100  /* Local = Message originated here     */
#define UPL_NETHOLD         0x0200  /* Hold = Hold for pickup, do not send */
#define UPL_NETIMMEDIATE    0x0400  /* Immediate = Send message NOW        */
#define UPL_NETFRQ          0x0800  /* File Request = Request file(s)      */
                                    /*   listed in Subject field           */
#define UPL_NETDIRECT       0x1000  /* Direct = Send direct, no routing    */
#define UPL_NRES6           0x2000  /* RESERVED FOR FUTURE USE             */
#define UPL_NRES7           0x4000  /* RESERVED FOR FUTURE USE             */
#define UPL_NETURQ          0x8000  /* Update Request = Request updated    */
                                    /*   file(s) listed in Subject field   */

/*  Values for UPL_REC.NETWORK_TYPE field  */

#define UPL_NET_FIDONET     0   /* FidoNet-style E-mail and conferences     */
                                /*   UPL_NETMAIL=off - Local, Echo, Group   */
                                /*   UPL_NETMAIL=on  - NetMail              */
#define UPL_NET_INTERNET    1   /* Internet E-mail and Usenet newsgroups    */
                                /*   UPL_NETMAIL=off - Local, Newsgroup     */
                                /*   UPL_NETMAIL=on  - E-mail               */

typedef struct      /*  UPL_REC  */
{
    tBYTE from[36];         /* Person message is from                        */
                            /*   NOTE: Doors should validate this field!     */
    tBYTE to[36];           /* Person message is to (non-Internet)           */
                            /*   For Internet E-mail, the NET_DEST field     */
                            /*   should be used to store the destination     */
                            /*   name/address, leaving this field blank.     */
                            /*   For Usenet newsgroups, this field should be */
                            /*   left blank, as newsgroups don't use a "To:" */
                            /*   field.                                      */
    tBYTE subj[72];         /* Subject/Title of message                      */
    tWORD destzone;         /* Destination address (FidoNet only)            */
                            /*   If the message is not a FidoNet NetMail     */
                            /*   message, this field (and the subsequent     */
                            /*   three fields as well) should be set to      */
                            /*   zero.                                       */
    tWORD destnet;
    tWORD destnode;
    tWORD destpoint;
    tWORD msg_attr;         /* Bit-mapped message attributes                 */
    tWORD netmail_attr;     /* Bit-mapped NetMail attributes (FidoNet only)  */
                            /*   If the message is not a FidoNet NetMail     */
                            /*   message, this field should not be used.     */
    tLONG unix_date;        /* Date/time of message                          */
                            /*   This Unix-style date/time value (number     */
                            /*   of seconds since 01/01/70) is converted to  */
                            /*   the date/time storage method used by the    */
                            /*   host BBS.                                   */
    tDWORD replyto;         /* This unsigned long word stores the message #  */
                            /*   that this message is a reply to.  This      */
                            /*   should be the same as FTI.MSGNUM.  Note,    */
                            /*   however, that FTI.MSGNUM is a word.  C      */
                            /*   programmers especially will need to         */
                            /*   properly typecast the value (i.e.           */
                            /*   upl.replyto=(tDWORD)fti.msgnum).  As        */
                            /*   messaging/BBS systems become more complex,  */
                            /*   FTI.MSGNUM may become obsolete, and a       */
                            /*   tDWORD variable may be used in its place.   */
    tBYTE filename[13];     /* Filename the message text is in               */
                            /*   If this file does not exist in the upload   */
                            /*   packet then doors should consider this an   */
                            /*   invalid record.                             */
    tBYTE echotag[21];      /* Area tag the message goes in                  */
                            /*   This must correspond exactly to the         */
                            /*   INF_AREA_INFO.ECHOTAG field for the message */
                            /*   area this message belongs to.  Simple area  */
                            /*   number matching has proven not to work      */
                            /*   simply because sysops are finicky people,   */
                            /*   and seem to constantly renumber/change the  */
                            /*   message area numbers on the host BBS.       */
                            /*   Using an echotag helps to alleviate this    */
                            /*   problem.  C_ECHO will be C_ECHO on the BBS, */
                            /*   whether it is msg area 17 on the host BBS   */
                            /*   or whether it is area 207. Doors should do  */
                            /*   a case-INSENSITIVE compare on this field to */
                            /*   find where the message belongs.             */
    tWORD area_flags;       /* The Blue Wave Offline Mail Reader uses this   */
                            /*   word internally to store the same value as  */
                            /*   in INF_AREA_INFO.AREA_FLAGS.  The purpose   */
                            /*   of this word is to hold the original        */
                            /*   information about the message area so that  */
                            /*   later message editing processes can be      */
                            /*   controlled properly.  For example, if a     */
                            /*   user later wanted to edit this message, the */
                            /*   reader would know instantly whether this is */
                            /*   a NETMAIL area, whether PVT messages are    */
                            /*   allowed, etc.  This allows re-editing of    */
                            /*   the message, even when there is not a       */
                            /*   corresponding *.INF file laying around, or  */
                            /*   the area is not listed in the *.INF file    */
                            /*   you currently have to work with.  DOOR      */
                            /*   AUTHORS SHOULD IGNORE THIS FIELD WHEN       */
                            /*   IMPORTING MESSAGES!                         */
    tBYTE f_attach[13];     /* If the UPL_HAS_FILE flag is set, this field   */
                            /*   will contain the file name that is attached */
                            /*   to the message.                             */
    tBYTE user_area[6];     /* User-defined storage.  Doors should ignore    */
                            /*   this field, and reader authors should feel  */
                            /*   free to utilize this field for their own    */
                            /*   internal use, if necessary.                 */
    tBYTE network_type;     /* Indicates the network type.  This field must  */
                            /*   hold the same value as the NETWORK_TYPE     */
                            /*   field in INF_AREA_INFO, allowing doors and  */
                            /*   readers to properly handle the message.     */
                            /*   (Values duplicated as UPL_NET_xxx, above.)  */
                            /*   For FidoNet NetMail and Internet E-mail, it */
                            /*   also indicates which fields should be used  */
                            /*   for addressing and status information (as   */
                            /*   indicated in comments above and below).     */
    tBYTE net_dest[100];    /* Network destination address (non-FidoNet)     */
                            /*   Internet E-mail messages should use this    */
                            /*   field to store the destination address.     */
}
UPL_REC;

/*---------------------------------------------------------------------------*/

/*
**  Name of file:   *.REQ
**
**  Description:    The *.REQ file is simply a list of filenames the user
**                  wants to request from the host BBS.  Wildcard characters
**                  ("*" and "?" under MS-DOS) are allowed, but are not
**                  guaranteed to produce accurate results on all door
**                  implementations.
**
**                  NOTE:   Current Blue Wave doors do not accept wildcard
**                          characters in filenames, and will consider any
**                          filenames which contain them as being invalid.
**                          Additionally, if there are more than 10 entries in
**                          the *.REQ file, current Blue Wave doors will read
**                          the first 10 and discard the rest.  These are
**                          limitations of the Blue Wave doors, not of the
**                          Blue Wave format itself.
**
**  File format:    REQ_REC     { repeated for as many files as }
**                  REQ_REC     { requested from the host BBS   }
**                  ...
*/

typedef struct      /*  REQ_REC  */
{
    tBYTE filename[13];     /* Name of file to request */
}
REQ_REC;

/*---------------------------------------------------------------------------*/

/*
**  Name of file:   *.PDQ
**
**  Description:    The *.PDQ file contains the information used for the
**                  offline configuration feature of the mail door.  After the
**                  header is a series of records which indicate the message
**                  areas to enable for scanning the next time a mail packet
**                  is requested.
**
**                  NOTE:   Readers should generate a *.PDQ file *ONLY* if
**                          the mail packet format is version 2 or earlier;
**                          otherwise, a *.OLC file should be generated.
**                          Doors should process *.PDQ files *ONLY* in cases
**                          where a *.OLC file is not present.
**
**                          If the AREA_CHANGES flag in PDQ_HEADER.FLAGS is
**                          set, the door should process the offline
**                          configuration as well as changes to the list of
**                          areas the user wants to download.  In the Blue
**                          Wave door, this is done by first turning OFF all
**                          message areas that were active, then turning ON
**                          the ones specified in the *.PDQ file.  This seems
**                          to be the simplest, most straight-forward method
**                          of accomplishing this task, though other, more
**                          complex schemes could easily have been devised.
**
**  File format:    PDQ_HEADER      { only included one time!
**                  PDQ_REC         { repeated for as many message areas }
**                  PDQ_REC         { as the user wishes to enable       }
**                  ...
*/

/*  Bit-masks for PDQ_HEADER.FLAGS field  */

#define PDQ_HOTKEYS         0x0001  /* Toggle "hotkeys" in prompts        */
#define PDQ_XPERT           0x0002  /* Toggle expert mode (menu displays) */
#define PDQ_AREA_CHANGES    0x0004  /* Change active message areas        */
#define PDQ_GRAPHICS        0x0008  /* Toggle IBM 8-bit ASCII characters  */
#define PDQ_NOT_MY_MAIL     0x0010  /* Toggle bundling mail from user     */

typedef struct      /*  PDQ_HEADER  */
{
    tBYTE keywords[10][21];     /* User's entire set of door keywords  */
    tBYTE filters[10][21];      /* User's entire set of door filters   */
    tBYTE macros[3][78];        /* User's door bundling command macros */
    tBYTE password[21];         /* Password                            */
    tBYTE passtype;             /* Password type                       */
                                /*   0=none 1=door 2=reader 3=both     */
    tWORD flags;                /* Bit-mapped flags                    */
}
PDQ_HEADER;

typedef struct      /*  PDQ_REC  */
{
    tBYTE echotag[21];      /* Echo tag of message area to activate    */
                            /*   With Telegard systems, this should    */
                            /*   be the name of the *.BRD file, rather */
                            /*   than the actual echo tag.             */
}
PDQ_REC;

/*---------------------------------------------------------------------------*/

/*
**  Name of file:   *.OLC
**
**  Description:    The *.OLC file contains the information used for the
**                  offline configuration feature of the mail door.
**
**                  NOTE:   Readers should generate a *.OLC file *ONLY* if
**                          the mail packet format is version 3 or later;
**                          otherwise, a *.PDQ file should be generated.
**                          Doors should process *.OLC files in all cases
**                          where one is present.
**
**  File format:    ASCII text (lines terminated with CRLF)
**
**  Comments:       Refer to the Blue Wave Developer's Kit documentation
**                  for details on the exact format of the *.OLC file.
*/

/*---------------------------------------------------------------------------*/

#endif      /*  __BLUEWAVE_H  */

