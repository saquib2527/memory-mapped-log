#ifndef MEMORY_MAPPED_LOG_H_
#define MEMORY_MAPPED_LOG_H_

#define MEMORY_MAPPED_LOG_NAME_SIZE 50

class Meta{
	public:
		char name[MEMORY_MAPPED_LOG_NAME_SIZE];
		unsigned long element_size;
		unsigned long history_size;
		unsigned long current_seq;

		Meta();
		Meta(const char *name, unsigned long element_size = 0, unsigned long history_size = 0, unsigned long current_seq = 0);
		unsigned char *serialize(unsigned char *buffer);
		unsigned char *deserialize(unsigned char *buffer);
		static unsigned long get_serialized_length();
};

class MemoryMappedLog{
	private:
		static void resolve_name(char *buffer, const char *file, const char *folder);
	public:
		static int save_meta(Meta &meta, const char *file, const char *folder);
		static int load_meta(Meta &meta, const char *file, const char *folder);
		static int create(const char *file, const char *folder, unsigned long element_size, unsigned long history_size);
		static unsigned long append(unsigned char * buffer, const char *file, const char *folder);
		static int read_seq(unsigned char *buffer, unsigned long seq_no, const char *file, const char *folder);
		static unsigned long get_latest_seq(const char *file, const char *folder);
		static int read_last(unsigned char *buffer, const char *file, const char *folder);
};

#endif
