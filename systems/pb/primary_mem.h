

typedef struct _PrimaryMemManager PrimaryMemManager;

struct _PrimaryMemManager {
	char			tmpfs_file_name[30];
	bool			a_free;
	bool			b_free;
	int				size;
	//int				offset;
	//DirectLink		*chunks;
	void			*v_addr; 			/*virtual address mapped into the process address space*/
	//int				avail;
}

/*typedef struct _Chunk Chunk;

struct _Chunk {

}*/

DFBResult pb_primarymemmanager_create(char* filename, PrimaryMemManager *pmm);

DFBResult pb_primarymemmanager_destroy(PrimaryMemManager *pmm);

DFBResult pb_primarymemmanager_allocate(PrimaryMemManager *pmm, void **ret_addr);

DFBResult pb_primarymemmanager_deallocate(PrimaryMemManager *pmm, void *addr);


