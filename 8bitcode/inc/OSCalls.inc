
; Error Codes
ENOENT  =  2 ; No such file or directory
EIO     =  5 ; I/O error
E2BIG   =  7 ; Arg list too long
EBADF   =  9 ; Bad file number
EAGAIN	= 11 ; Resource temporarily unavailable
ENOMEM  = 12 ; Not enough core memory
EACCES  = 13 ; Permission denied
EEXIST  = 17 ; File exists
ENOTDIR = 20 ; Not a directory
EISDIR  = 21 ; Is a directory
EINVAL  = 22 ; Invalid argument
ENFILE  = 23 ; File table overflow
EMFILE  = 24 ; Too many open files
ENOSPC  = 28 ; No space left on device
EROFS   = 30 ; Read only file system
EPIPE	= 32 ; Broken pipe
ENOTSUP = 48 ; Operation not supported
EEOF    = 240 ; End of File condition
ERELOC  = 241 ; program relocation error


; Open Mode Flags
O_READ     =  1 ; open for reading
O_WRITE    =  2 ; open for writing
O_NONBLOCK =  4 ; non-blocking mode
O_APPEND   =  8 ; append to end of file
O_CREATE   = 16 ; create if non existing
O_DIR      = 32 ; must be a directory
O_LISTALL  =  4 ; list all directory entries

; FAT Directory Entry Offsets
FAT_DIRENT_NAME = 0
FAT_DIRENT_ATTR = 11
FAT_DIRENT_NTRES = 12
FAT_DIRENT_CRTTIMETENTH = 13
FAT_DIRENT_CRTTIME = 14
FAT_DIRENT_CRTDATE = 16
FAT_DIRENT_LSTACCDATE = 18
FAT_DIRENT_FSTCLUSHI = 20
FAT_DIRENT_WRTTIME = 22
FAT_DIRENT_WRTDATE = 24
FAT_DIRENT_FSTCLUSLO = 26
FAT_DIRENT_FILESIZE = 28

; FAT directory entry attribute values (FAT_DIRENT_ATTR)
FAT_DIR_ATTR_READ_ONLY =  1 ; read only file
FAT_DIR_ATTR_HIDDEN    =  2 ; hidden file
FAT_DIR_ATTR_SYSTEM    =  4 ; system file
FAT_DIR_ATTR_VOLUME_ID =  8 ; volume name entry
FAT_DIR_ATTR_DIRECTORY = 16 ; sub directory entry
FAT_DIR_ATTR_ARCHIVE   = 32 ; file changed
FAT_DIR_ATTR_LONG_NAME = 15 ; part of long name

; FAT File System Status Offsets
FAT_FSSTAT_TOTAL = 0 ; total clusters
FAT_FSSTAT_FREE  = 2 ; free clusters
FAT_FSSTAT_SPC   = 4 ; sectors per cluster


OS_DISPLAY_CHAR              = 0xfff5
OS_DISPLAY_STRING            = OS_DISPLAY_CHAR - 3
OS_DISPLAY_HEX_BYTE          = OS_DISPLAY_STRING - 3
OS_DISPLAY_GOTO_LINE         = OS_DISPLAY_HEX_BYTE - 3
OS_DISPLAY_GOTO_POSITION     = OS_DISPLAY_GOTO_LINE - 3
OS_DISPLAY_GOTO_OFFSET       = OS_DISPLAY_GOTO_POSITION - 3
OS_DISPLAY_MOVE_CURSOR_RIGHT = OS_DISPLAY_GOTO_OFFSET - 3
OS_DISPLAY_MOVE_CURSOR_LEFT  = OS_DISPLAY_MOVE_CURSOR_RIGHT - 3
OS_DISPLAY_CLEAR             = OS_DISPLAY_MOVE_CURSOR_LEFT - 3
OS_DISPLAY_GET_CURSOR_POS    = OS_DISPLAY_CLEAR - 3
OS_DISPLAY_GET_CURSOR_OFFSET = OS_DISPLAY_GET_CURSOR_POS - 3
OS_DISPLAY_READ_STATE        = OS_DISPLAY_GET_CURSOR_OFFSET - 3
OS_DISPLAY_READ_BYTE         = OS_DISPLAY_READ_STATE - 3
OS_DISPLAY_SET_BACKLIGHT     = OS_DISPLAY_READ_BYTE - 3
OS_DISPLAY_SEND_DATA_BYTE    = OS_DISPLAY_SET_BACKLIGHT - 3
OS_KEYBOARD_WAIT_FOR_KEY     = OS_DISPLAY_SEND_DATA_BYTE - 3
OS_CLOSE                     = OS_KEYBOARD_WAIT_FOR_KEY - 3
OS_READ                      = OS_CLOSE - 3
OS_OPEN                      = OS_READ - 3
OS_CLOSE_DIR                 = OS_OPEN - 3
OS_NEXT_DIR                  = OS_CLOSE_DIR - 3
OS_OPEN_DIR                  = OS_NEXT_DIR - 3
OS_GET_TIME                  = OS_OPEN_DIR - 3
OS_SET_TIME                  = OS_GET_TIME - 3
OS_GET_VERSION               = OS_SET_TIME - 3
OS_KEYBOARD_GET_KEY          = OS_GET_VERSION - 3
OS_DISPLAY_SEND_COMMAND      = OS_KEYBOARD_GET_KEY - 3
OS_WRITE                     = OS_DISPLAY_SEND_COMMAND - 3
OS_SEEK                      = OS_WRITE - 3
OS_TELL                      = OS_SEEK - 3
OS_MKDIR                     = OS_TELL - 3
OS_UNLINK                    = OS_MKDIR - 3
OS_STAT                      = OS_UNLINK - 3
OS_MEM_ALLOC                 = OS_STAT - 3
OS_MEM_FREE                  = OS_MEM_ALLOC - 3
OS_FSSTAT                    = OS_MEM_FREE - 3
OS_PUTC                      = OS_FSSTAT - 3
OS_GETC                      = OS_PUTC - 3
OS_EXEC                      = OS_GETC - 3
OS_FORK                      = OS_EXEC - 3
OS_WAIT_TASK                 = OS_FORK - 3
OS_GET_ENV_POINTER           = OS_WAIT_TASK - 3
OS_GET_TICKS                 = OS_GET_ENV_POINTER - 3
OS_SLEEP                     = OS_GET_TICKS - 3
OS_GET_TASK_LIST             = OS_SLEEP - 3
OS_GET_TASK_INFO             = OS_GET_TASK_LIST - 3
OS_SEND_TASK_SIGNAL          = OS_GET_TASK_INFO - 3