#ifndef EMBEXT2_H
#define EMBEXT2_H 1

#define MAX_PATH_LEN 1024
#define MAX_PATH_LEVELS 100

// reserved inode definitions
#define EXT2_BAD_INO 1
#define EXT2_ROOT_INO 2
#define EXT2_ACL_IDX_INO 3
#define EXT2_ACL_DATA_INO 4
#define EXT2_BOOT_LOADER_INO 5
#define EXT2_UNDEL_DIR_INO 6

#define EXT2_FLAG_OPEN 1
#define EXT2_FLAG_READ 2
#define EXT2_FLAG_WRITE 4
#define EXT2_FLAG_APPEND 8
#define EXT2_FLAG_DIRTY 16
#define EXT2_FLAG_FS_DIRTY 32

#define EXT2_S_IFDIR 0x4000

#define EXT2_FEATURE_RO_COMPAT_SPARSE_SUPER 0x0001
#define EXT2_FEATURE_RO_COMPAT_LARGE_FILE   0x0002
#define EXT2_FEATURE_RO_COMPAT_BTREE_DIR    0x0004

#define EXT2_ALLOCATED          1
#define EXT2_DEALLOCATED        0
struct superblock {
    uint32_t s_inodes_count;
    uint32_t s_blocks_count;
    uint32_t s_r_blocks_count;
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    uint32_t s_first_data_block;
    uint32_t s_log_block_size;
    uint32_t s_log_frag_size;
    uint32_t s_blocks_per_group;
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;
    uint32_t s_mtime;
    uint32_t s_wtime;
    uint16_t s_mnt_count;
    uint16_t s_max_mnt_count;
    uint16_t s_magic;
    uint16_t s_state;
    uint16_t s_errors;
    uint16_t s_minor_rev_level;
    uint32_t s_lastcheck;
    uint32_t s_checkinterval;
    uint32_t s_creator_os;
    uint32_t s_rev_level;
    uint16_t s_def_resuid;
    uint16_t s_def_resgid;
    uint32_t s_first_ino;
    uint16_t s_inode_size;
    uint16_t s_block_group_nr;
    uint32_t s_feature_compat;
    uint32_t s_feature_incompat;
    uint32_t s_feature_ro_compat;
    uint8_t s_uuid[16];
    char s_volume_name[16];
    char s_last_mounted[64];
    uint32_t s_algo_bitmap;
    uint8_t s_prealloc_blocks;
    uint8_t s_prealloc_dir_blocks;
    uint8_t alignment1[2];
    uint8_t s_journal_uuid[16];
    uint32_t s_journal_inum;
    uint32_t s_journal_dev;
    uint32_t s_last_orphan;
    uint32_t s_hash_seed[4];
    uint8_t s_def_hash_version;
    uint8_t alignment2[3];
    uint32_t s_default_mount_options;
    uint32_t s_first_meta_bg;
} __attribute__((__packed__));

struct block_group_descriptor {
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table;
    uint16_t bg_free_blocks_count;
    uint16_t bg_free_inodes_count;
    uint16_t bg_used_dirs_count;
    uint16_t bg_pad;
} __attribute__((__packed__));

struct inode {
    uint16_t i_mode;
    uint16_t i_uid;
    uint32_t i_size;
    uint32_t i_atime;
    uint32_t i_ctime;
    uint32_t i_mtime;
    uint32_t i_dtime;
    uint16_t i_gid;
    uint16_t i_links_count;
    uint32_t i_blocks;
    uint32_t i_flags;
    uint32_t i_osd1;
    uint32_t i_block[15];
    uint32_t i_generation;
    uint32_t i_file_acl;
    uint32_t i_dir_acl;
    uint32_t i_faddr;
    uint8_t i_osd2[12];
} __attribute__((__packed__));

struct ext2context {
    blockno_t part_start;
    struct superblock superblock;
    uint32_t sparse;
    uint8_t sysbuf[512];
    uint32_t superblock_block;
    uint32_t read_only;
    uint32_t num_blockgroups;
    uint32_t num_superblocks;
    uint32_t *superblock_blocks;
};

struct ext2_dirent {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name;
};

struct file_ent {
    struct ext2context *context;
    uint32_t flags;
    uint32_t cursor;
    uint32_t inode_number;
    uint32_t sector;
    uint32_t file_sector;
    uint32_t sectors_left;
    uint32_t block_index[3];
    uint8_t buffer[512];
    struct inode inode;
};

int ext2_mount(blockno_t part_start, blockno_t volume_size, uint8_t filesystem_hint, struct ext2context **context);

struct file_ent *ext2_open(struct ext2context *context, const char *name, int flags, int mode, int *rerrno);

int ext2_close(struct file_ent *fe, int *rerrno);

int ext2_read(struct file_ent *fe, void *buffer, size_t count, int *rerrno);

int ext2_write(struct file_ent *fe, const void *buffer, size_t count, int *rerrno);

int ext2_isatty(struct file_ent *fe, int *rerrno);

int ext2_fstat(struct file_ent *fe, struct stat *st, int *rerrno);

int ext2_lseek(struct file_ent *fe, int ptr, int dir, int *rerrno);

struct dirent *ext2_readdir(struct file_ent *fe, int *rerrno);

#endif /* ifndef EMBEXT2_H */
