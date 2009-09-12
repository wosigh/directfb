#include "primary_mem.h"

#define PRIMARY_MEM_SIZE 2*1024*1024

DFBResult pb_primarymemmanager_create(char* filename, PrimaryMemManager **pmm)
{
	DFBResult ret;
	
	PrimaryMemManager ret_pmm = *pmm
	ret_pmm = D_CALLOC(1, sizeof(PrimaryMemManager));
	if (!ret_pmm)
		return D_OOSHM();
	
	int fd;
	fd = open(buf, O_RDWR | O_CREAT | O_TRUNC, 0660);
	if (fd < 0) {
		D_PERROR( "Primary Mem/Init: Couldn't open shared memory file!\n" );
		ret = DFB_INIT;
		goto error;
    }
	fchmod( fd, 0660 );
	ftruncate( fd, PRIMARY_MEM_SIZE );
	
	/* Map */
	ret_pmm->v_addr = mmap( NULL, PRIMARY_MEM_SIZE 
						PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, fd, 0 );
	if (ret_pmm->v_addr == MAP_FAILED) {
		D_PERROR( "Fusion/Init: Mapping Primary SHM file failed!\n" );
		close( fd );
		ret = DFB_INIT;
		goto error;
	}      
	close( fd );

	ret_pmm->size = PRIMARY_MEM_SIZE;
	ret_pmm->a_free = true;
	ret_pmm->b_free = true;
	strcpy(ret_pmm->tmpfs_file_name, filename);

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
	if (pmm->a_free) {
		*ret_addr = pmm->v_addr;
		pmm->a_free = false;
	} 
	else if (pmm->b_free) {
		*ret_addr = pmm->v_addr + pmm->size/2;
		pmm->b_free = false;
	}
	else {
		D_ERROR("No more primary mem chunk! Maybe 2 is small\n");
		return DFB_FAILURE;
	
	return DFB_OK;
}

DFBResult pb_primarymemmanager_deallocate(PrimaryMemManager *pmm, void *addr)
{
	if (addr == pmm->v_addr)
		pmm->a_free = true;
	else if (addr == pmm->v_addr + pmm->size/2)
		pmm->b_free = true;
	else {
		D_ERROR("No such address in primary mem\n");
		return DFB_FAILURE;
	}	

	return DFB_OK;
}

