#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "MemoryMappedFile.h"
#include "MemoryMappedLog.h"
#include "Serializer.h"

Meta::Meta(){
	std::memset(this->name, 0, MEMORY_MAPPED_LOG_NAME_SIZE);
	this->element_size = 0;
	this->history_size = 0;
	this->current_seq = 0;
}

Meta::Meta(const char *name, unsigned long element_size, unsigned long history_size, unsigned long current_seq){
	std::memset(this->name, 0, MEMORY_MAPPED_LOG_NAME_SIZE);
	std::strcpy(this->name, name);
	this->element_size = element_size;
	this->history_size = history_size;
	this->current_seq = current_seq;
}

unsigned char *Meta::serialize(unsigned char *buffer){

	int i;
	unsigned char *end;

	end = buffer;

	for(i = 0; i < MEMORY_MAPPED_LOG_NAME_SIZE; ++i){
		end = Serializer::serialize_char(end, this->name[i]);
	}
	end = Serializer::serialize_ulong(end, this->element_size);
	end = Serializer::serialize_ulong(end, this->history_size);
	end = Serializer::serialize_ulong(end, this->current_seq);

	return end;

}

unsigned char *Meta::deserialize(unsigned char *buffer){

	int i;
	unsigned char *end;

	end = buffer;

	for(i = 0; i < MEMORY_MAPPED_LOG_NAME_SIZE; ++i){
		end = Serializer::deserialize_char(end, &this->name[i]);
	}
	end = Serializer::deserialize_ulong(end, &this->element_size);
	end = Serializer::deserialize_ulong(end, &this->history_size);
	end = Serializer::deserialize_ulong(end, &this->current_seq);

	return end;

}

unsigned long Meta::get_serialized_length(){

	return
		MEMORY_MAPPED_LOG_NAME_SIZE +
		sizeof(unsigned long) +
		sizeof(unsigned long) +
		sizeof(unsigned long);

}

void MemoryMappedLog::resolve_name(char *buffer, const char *file, const char *folder){

	std::memset(buffer, 0, MEMORY_MAPPED_LOG_NAME_SIZE);
	if(folder == NULL) std::sprintf(buffer, "%s", file);
	else std::sprintf(buffer, "%s/%s", folder, file);

}

int MemoryMappedLog::save_meta(Meta &meta, const char *file, const char *folder){

	char log_name[MEMORY_MAPPED_LOG_NAME_SIZE];
	int status;
	unsigned char *buffer;

	MemoryMappedLog::resolve_name(log_name, file, folder);

	buffer = (unsigned char *)calloc(Meta::get_serialized_length(), sizeof(unsigned char));
	meta.serialize(buffer);
	status = MemoryMappedFile::write_to_file(log_name, buffer, Meta::get_serialized_length(), 0);
	free(buffer);

	return status;

}

int MemoryMappedLog::load_meta(Meta &meta, const char *file, const char *folder){

	char log_name[MEMORY_MAPPED_LOG_NAME_SIZE];
	int status;
	unsigned char *buffer;

	MemoryMappedLog::resolve_name(log_name, file, folder);

	buffer = (unsigned char *)calloc(Meta::get_serialized_length(), sizeof(unsigned char));
	status = MemoryMappedFile::read_from_file(log_name, buffer, Meta::get_serialized_length(), 0);
	meta.deserialize(buffer);
	free(buffer);

	return status;

}

int MemoryMappedLog::create(const char *file, const char *folder, unsigned long element_size, unsigned long history_size){

	char log_name[MEMORY_MAPPED_LOG_NAME_SIZE];
	Meta meta(file, element_size, history_size);
	unsigned long size;

	MemoryMappedLog::resolve_name(log_name, file, folder);

	size = Meta::get_serialized_length() + history_size * element_size;
	if(MemoryMappedFile::create_empty_file(log_name, size) == -1) return -1;

	return MemoryMappedLog::save_meta(meta, file, folder);

}

unsigned long MemoryMappedLog::append(unsigned char *buffer, const char *file, const char *folder){

	char log_name[MEMORY_MAPPED_LOG_NAME_SIZE];
	Meta meta;
	unsigned long offset;

	MemoryMappedLog::resolve_name(log_name, file, folder);
	if(MemoryMappedLog::load_meta(meta, file, folder) == -1) return (unsigned long)-1;
	
	offset = Meta::get_serialized_length() + meta.element_size * meta.current_seq;
	if(MemoryMappedFile::write_to_file(log_name, buffer, meta.element_size, offset) == -1) return (unsigned long)-1;

	++meta.current_seq;
	if(MemoryMappedLog::save_meta(meta, file, folder) == -1) return (unsigned long)-1;

	return meta.current_seq;

}

int MemoryMappedLog::read_seq(unsigned char *buffer, unsigned long seq_no, const char *file, const char *folder){

	char log_name[MEMORY_MAPPED_LOG_NAME_SIZE];
	Meta meta;
	unsigned long offset;

	MemoryMappedLog::resolve_name(log_name, file, folder);
	if(MemoryMappedLog::load_meta(meta, file, folder) == -1) return -1;
	
	offset = Meta::get_serialized_length() + meta.element_size * (seq_no - 1);
	if(MemoryMappedFile::read_from_file(log_name, buffer, meta.element_size, offset) == -1) return -1;

	return meta.current_seq;

}

unsigned long MemoryMappedLog::get_latest_seq(const char *file, const char *folder){

	Meta meta;

	if(MemoryMappedLog::load_meta(meta, file, folder) == -1) return (unsigned long)-1;
	return meta.current_seq;

}

int MemoryMappedLog::read_last(unsigned char *buffer, const char *file, const char *folder){

	char log_name[MEMORY_MAPPED_LOG_NAME_SIZE];
	Meta meta;
	unsigned long offset;

	MemoryMappedLog::resolve_name(log_name, file, folder);
	if(MemoryMappedLog::load_meta(meta, file, folder) == -1) return -1;
	if(meta.current_seq == 0) return 1;
	
	offset = Meta::get_serialized_length() + meta.element_size * (meta.current_seq - 1);
	if(MemoryMappedFile::read_from_file(log_name, buffer, meta.element_size, offset) == -1) return -1;

	return 0;

}
