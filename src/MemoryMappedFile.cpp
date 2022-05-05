#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "MemoryMappedFile.h"

int MemoryMappedFile::create_empty_file(const char *fname, unsigned long size){

	FILE *fp;

	if(!(fp = std::fopen(fname, "w"))) return -1;
	if(ftruncate(fileno(fp), size) == -1) return -1;
	if(std::fclose(fp)) return -1;

	return 0;

}

int MemoryMappedFile::read_from_file(const char *fname, unsigned char *buffer, unsigned long size, unsigned long offset){

	int fd;
	int status;
	struct stat sb;
	void *mmapret;

	if((fd = open(fname, O_RDWR, S_IRWXO)) == -1) return -1;

	if(fstat(fd, &sb) == -1){
		close(fd);
		return -1;
	}

	mmapret = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(mmapret == (void *)-1){
		close(fd);
		return -1;
	}

	std::memcpy(buffer, (unsigned char *)mmapret + offset, size);

	status = munmap(mmapret, sb.st_size);
	if(status == -1){
		close(fd);
		return -1;
	}

	close(fd);

	return 0;

}

int MemoryMappedFile::write_to_file(const char *fname, unsigned char *buffer, unsigned long size, unsigned long offset){

	int fd;
	int status;
	struct stat sb;
	void *mmapret;

	if((fd = open(fname, O_RDWR, S_IRWXO)) == -1) return -1;

	if(fstat(fd, &sb) == -1){
		close(fd);
		return -1;
	}

	mmapret = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(mmapret == (void *)-1){
		close(fd);
		return -1;
	}

	memcpy((unsigned char *)mmapret + offset, buffer, size);

	status = munmap(mmapret, sb.st_size);
	if(status == -1){
		close(fd);
		return -1;
	}

	close(fd);

	return 0;

}
