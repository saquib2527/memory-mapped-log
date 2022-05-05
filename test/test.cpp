#include <sys/stat.h>
#include <gtest/gtest.h>

#include "MemoryMappedFile.h"
#include "MemoryMappedLog.h"

constexpr char fname[] = {"abc"};
constexpr int size = 100;

TEST(MemoryMappedFileTest, CreatesEmptyFile){

	char cmd[100];
	struct stat buf;

	sprintf(cmd, "rm -f %s", fname);
	system(cmd);

	MemoryMappedFile::create_empty_file(fname, size);

	ASSERT_EQ(0, stat(fname, &buf));
	ASSERT_EQ(buf.st_size, size);

	system(cmd);

}

TEST(MemoryMappedFileTest, ReadsWritesCorrectly){

	char cmd[100];
	unsigned char writer[] = {'a', 'b', 'c'};
	unsigned char reader[100];
	
	MemoryMappedFile::create_empty_file(fname, size);

	MemoryMappedFile::write_to_file(fname, writer, 3, 0);
	MemoryMappedFile::read_from_file(fname, reader, 1, 0);
	ASSERT_EQ(reader[0], 'a');
	MemoryMappedFile::read_from_file(fname, reader, 1, 1);
	ASSERT_EQ(reader[0], 'b');
	MemoryMappedFile::read_from_file(fname, reader, 1, 2);
	ASSERT_EQ(reader[0], 'c');

	MemoryMappedFile::write_to_file(fname, writer, 3, 0);
	MemoryMappedFile::read_from_file(fname, reader, 3, 0);
	ASSERT_EQ(reader[0], 'a');
	ASSERT_EQ(reader[1], 'b');
	ASSERT_EQ(reader[2], 'c');

	MemoryMappedFile::write_to_file(fname, writer, 3, 3);
	MemoryMappedFile::read_from_file(fname, reader, 3, 3);
	ASSERT_EQ(reader[0], 'a');
	ASSERT_EQ(reader[1], 'b');
	ASSERT_EQ(reader[2], 'c');

	sprintf(cmd, "rm -f %s", fname);
	system(cmd);

}

TEST(MetaTest, SetsDefaultValues){

	Meta meta;

	ASSERT_EQ(meta.name[0], 0);
	ASSERT_EQ(meta.element_size, 0);
	ASSERT_EQ(meta.history_size, 0);
	ASSERT_EQ(meta.current_seq, 0);

}

TEST(MetaTest, SetsGivenValues){

	char name[] = {"abc"};
	Meta meta(name, 4, 10);

	ASSERT_EQ(meta.name[0], 'a');
	ASSERT_EQ(meta.name[1], 'b');
	ASSERT_EQ(meta.name[2], 'c');
	ASSERT_EQ(meta.name[3], 0);
	ASSERT_EQ(meta.element_size, 4);
	ASSERT_EQ(meta.history_size, 10);
	ASSERT_EQ(meta.current_seq, 0);

}

TEST(MetaTest, GetsCorrectLength){

	int expected = sizeof(unsigned long) + sizeof(unsigned long) + sizeof(unsigned long) + MEMORY_MAPPED_LOG_NAME_SIZE;

	ASSERT_EQ(expected, Meta::get_serialized_length());

}

TEST(MetaTest, SerializesDeserializesCorrectly){

	char name[] = {"abc"};
	char changed_name[] = {"def"};
	Meta meta(name, 4, 10);
	size_t size;
	unsigned char buffer[100];
	unsigned char *end;

	end = buffer;
	end = meta.serialize(end);

	size = end - buffer;
	ASSERT_EQ(size, Meta::get_serialized_length());

	std::strcpy(meta.name, changed_name);
	meta.element_size = 78;
	meta.history_size = 89;
	meta.current_seq = 1009;

	end = buffer;
	end = meta.deserialize(end);

	ASSERT_EQ(std::strcmp(meta.name, name), 0);
	ASSERT_EQ(meta.element_size, 4);
	ASSERT_EQ(meta.history_size, 10);
	ASSERT_EQ(meta.current_seq, 0);

}

TEST(MetaTest, SavesLoadsCorrectly){

	char name[] = {"abc"};
	char cmd[100];
	Meta meta(name, 4, 10);

	std::sprintf(cmd, "rm -f %s", name);
	system(cmd);

	MemoryMappedFile::create_empty_file(name, Meta::get_serialized_length() + 10 * 4);

	MemoryMappedLog::save_meta(meta, name, nullptr);
	
	meta.element_size = 13;
	meta.history_size = 134;
	meta.current_seq = 98;

	MemoryMappedLog::load_meta(meta, name, nullptr);

	ASSERT_EQ(std::strcmp(meta.name, name), 0);
	ASSERT_EQ(meta.element_size, 4);
	ASSERT_EQ(meta.history_size, 10);
	ASSERT_EQ(meta.current_seq, 0);

	system(cmd);

}

TEST(MemoryMappedLogTest, CreatesLog){

	char name[] = {"abc"};
	char cmd[100];
	Meta meta(name, 4, 10);
	size_t size;
	struct stat buf;

	std::sprintf(cmd, "rm -f %s", name);
	system(cmd);

	MemoryMappedLog::create(name, nullptr, meta.element_size, meta.history_size);

	ASSERT_EQ(stat(name, &buf), 0);
	size = buf.st_size;
	ASSERT_EQ(size, (Meta::get_serialized_length() + 4 * 10));

	system(cmd);

}

TEST(MemoryMappedLogTest, GetsLatestSeq){

	char name[] = {"abc"};
	char cmd[100];
	Meta meta(name, 4, 10);

	MemoryMappedLog::create(name, nullptr, meta.element_size, meta.history_size);

	ASSERT_EQ(MemoryMappedLog::get_latest_seq(name, nullptr), 0);

	meta.current_seq = 10;
	MemoryMappedLog::save_meta(meta, name, nullptr);
	
	ASSERT_EQ(MemoryMappedLog::get_latest_seq(name, nullptr), 10);

	std::sprintf(cmd, "rm -f %s", name);
	system(cmd);

}

TEST(MemoryMappedLogTest, AppendsReadsCorrectly){

	char name[] = {"abc"};
	char cmd[100];
	unsigned char buffer[4] = {0b01000001, 0b01000010, 0b01000011, 0b01000100};
	Meta meta(name, 4, 10);

	MemoryMappedLog::create(meta.name, nullptr, meta.element_size, meta.history_size);
	MemoryMappedLog::append(buffer, meta.name, nullptr);

	std::memset(buffer, 0, sizeof(unsigned char) * 4);

	MemoryMappedLog::read_seq(buffer, 1, meta.name, nullptr);

	ASSERT_EQ(buffer[0], 0b01000001);
	ASSERT_EQ(buffer[1], 0b01000010);
	ASSERT_EQ(buffer[2], 0b01000011);
	ASSERT_EQ(buffer[3], 0b01000100);

	std::sprintf(cmd, "rm -f %s", name);
	system(cmd);

}

TEST(MemoryMappedLogTest, RespectsFolder){

	char cmd[100];
	char folder[] = {"working"};
	char name[] = {"abc"};
	char full_name[100];
	struct stat buf;
	size_t size;
	unsigned char buffer[4] = {0b01000001, 0b01000010, 0b01000011, 0b01000100};
	Meta meta(name, 4, 10);

	std::sprintf(cmd, "mkdir %s", folder);
	system(cmd);

	MemoryMappedLog::create(meta.name, folder, meta.element_size, meta.history_size);

	std::sprintf(full_name, "%s/%s", folder, name);
	ASSERT_EQ(stat(full_name, &buf), 0);

	size = buf.st_size;
	ASSERT_EQ(size, (Meta::get_serialized_length() + meta.element_size * meta.history_size));
	ASSERT_EQ(MemoryMappedLog::get_latest_seq(meta.name, folder), 0);

	MemoryMappedLog::append(buffer, meta.name, folder);

	std::memset(buffer, 0, sizeof(unsigned char) * 4);

	MemoryMappedLog::read_seq(buffer, 1, meta.name, folder);

	ASSERT_EQ(MemoryMappedLog::get_latest_seq(meta.name, folder), 1);
	ASSERT_EQ(buffer[0], 0b01000001);
	ASSERT_EQ(buffer[1], 0b01000010);
	ASSERT_EQ(buffer[2], 0b01000011);
	ASSERT_EQ(buffer[3], 0b01000100);

	std::sprintf(cmd, "rm -rf %s", folder);
	system(cmd);

}

TEST(MemoryMappedLogTest, ReadsLast){

	char cmd[100];
	char folder[] = {"working"};
	char name[] = {"abc"};
	Meta meta(name, 1, 10);
	unsigned char buffer[] = {0b00111100, 0b01010101, 0b11000011};

	std::sprintf(cmd, "mkdir %s", folder);
	system(cmd);

	MemoryMappedLog::create(name, folder, meta.element_size, meta.history_size);

	MemoryMappedLog::append(buffer, name, folder);
	MemoryMappedLog::append(&buffer[1], name, folder);
	MemoryMappedLog::append(&buffer[2], name, folder);

	ASSERT_EQ(buffer[0], 0b00111100);
	MemoryMappedLog::read_last(buffer, name, folder);
	ASSERT_EQ(buffer[0], 0b11000011);

	std::sprintf(cmd, "rm -rf %s", folder);
	system(cmd);

}

int main(int argc, char *argv[]){

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();

	return 0;

}
