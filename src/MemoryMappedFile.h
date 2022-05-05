#ifndef MEMORY_MAPPED_FILE_H_
#define MEMORY_MAPPED_FILE_H_

class MemoryMappedFile{
	public:
		static int create_empty_file(const char *fname, unsigned long size);
		static int read_from_file(const char *fname, unsigned char *buffer, unsigned long size, unsigned long offset);
		static int write_to_file(const char *fname, unsigned char *buffer, unsigned long size, unsigned long offset);
};

#endif
