#include <directfb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
      
#include <unistd.h>

#include "primary_mem.h"

#define PRIMARY_MEM_SIZE MAX_MEM_CHUNCKS*1024*1024

DFBResult pb_primarymemmanager_create(char* filename, PrimaryMemManager **pmm)
{
	DFBResult ret;
	int i;
	
	PrimaryMemManager *ret_pmm;
	ret_pmm = D_CALLOC(1, sizeof(PrimaryMemManager));
	if (!ret_pmm)
		return D_OOM();
	
	int fd;
	fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0660);
	if (fd < 0) {
		D_PERROR( "Primary Mem/Init: Couldn't open shared memory file!\n" );
		ret = DFB_INIT;
		goto error;
    }
	fchmod( fd, 0660 );
	ftruncate( fd, PRIMARY_MEM_SIZE );
	
	/* Map */
	ret_pmm->v_addr = mmap( NULL, PRIMARY_MEM_SIZE, 
						PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
	if (ret_pmm->v_addr == MAP_FAILED) {
		D_PERROR( "Fusion/Init: Mapping Primary SHM file failed!\n" );
		close( fd );
		ret = DFB_INIT;
		goto error;
	}      
	close( fd );

	ret_pmm->size = PRIMARY_MEM_SIZE;
	for (i=0; i<MAX_MEM_CHUNCKS; i++)
		ret_pmm->n_free[i] = true;

	strcpy(ret_pmm->tmpfs_file_name, filename);

	*pmm = ret_pmm;

	return DFB_OK;
	
error:
	D_FREE(pmm);
	return ret;
}

DFBResult pb_primarymemmanager_destroy(PrimaryMemManager *pmm)
{
	if (munmap(pmm->v_addr, pmm->size) < 0) {
		D_PERROR("munmap primary mem failed\n");
		return DFB_FAILURE;
	}
	D_FREE(pmm);
	return DFB_OK;
}

DFBResult pb_primarymemmanager_allocate(PrimaryMemManager *pmm, void **ret_addr)
{
	int i;

	for(i=0; i<MAX_MEM_CHUNCKS; i++) {
		if (pmm->n_free[i]) {
			*ret_addr = pmm->v_addr + i * (pmm->size/MAX_MEM_CHUNCKS);
			pmm->n_free[i] = false;
			break;
		}	
	}
	if (i == MAX_MEM_CHUNCKS) {
		D_ERROR("No more primary mem chunk! Maybe %d is small\n", MAX_MEM_CHUNCKS);
		printf("No more primary mem chunk! Maybe %d is small\n", MAX_MEM_CHUNCKS);
		return DFB_FAILURE;
	}	
	
	return DFB_OK;
}

DFBResult pb_primarymemmanager_deallocate(PrimaryMemManager *pmm, void *addr)
{
	int i;

	for(i=0; i<MAX_MEM_CHUNCKS; i++) {
		if (addr == pmm->v_addr + i * (pmm->size/MAX_MEM_CHUNCKS)) {
			pmm->n_free[i] = true;
			break;
		}
	}

	if (i == MAX_MEM_CHUNCKS) {
		D_ERROR("No such address in primary mem\n");
		return DFB_FAILURE;
	}	

	return DFB_OK;
}
