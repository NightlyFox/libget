// Copyright 2016 Zarklord1 : https://github.com/Zarklord1/WiiU_Save_Manager
#include <string.h>
#include <fcntl.h>
#include <string>
#include <dirent.h>
#include <stdio.h>
#include <cstdint>
#include <unistd.h>

#include "ZipUtil.hpp"
#include "Utils.hpp"

#define u32 uint32_t
#define u8 uint8_t

#define ONE_MB (1000000)
#define EIGHT_MB (8000000) 

using namespace std;

Zip::Zip(const char * zipPath) {
	//fileToZip = zipOpen(zipPath,APPEND_STATUS_CREATE);
	//if(fileToZip == NULL) printf("Error Opening: %s for zipping files!\n",zipPath);
	//memset(&fileToUnzip, 0, sizeof(fileToUnzip));
	//fileStatus = mz_zip_writer_init(fileToUnzip, );
}

Zip::~Zip() {
	Close();
}

int Zip::AddFile(const char * internalPath, const char * path) {
	//zipOpenNewFileInZip(fileToZip, internalPath, NULL, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
	printf("Adding %s to zip, under path %s\n",path,internalPath);
	int code = Add(path);
	//zipCloseFileInZip(fileToZip);
	return code;
}

int Zip::AddDir(const char * internalDir, const char * externalDir) {
	struct dirent *dirent = NULL;
	DIR *dir = NULL;

	dir = opendir(externalDir);
	if (dir == NULL) {
		return -1;
	}

	while ((dirent = readdir(dir)) != 0) {
		if(strcmp(dirent->d_name, "..") == 0 || strcmp(dirent->d_name, ".") == 0)
			continue;
		
		std::string zipPath(internalDir);
		zipPath += '/';
		zipPath += dirent->d_name;
		std::string realPath(externalDir);
		realPath += '/';
		realPath += dirent->d_name;

		if(dirent->d_type & DT_DIR) {
			AddDir(zipPath.c_str(), realPath.c_str());
		} else {
			AddFile(zipPath.c_str(),realPath.c_str());
		}
	}
	closedir(dir);
	return 0;
}

int Zip::Add(const char * path) {

	int fileNumber = open(path, O_RDONLY);
	if(fileNumber == -1) 
		return -1;
		
	u32 filesize = lseek(fileNumber, 0, SEEK_END);
	lseek(fileNumber, 0, SEEK_SET);
	
	size_t blocksize = 1024000;
	char buffer[blocksize];
	u32 done = 0;
	
	int readBytes = 0;

	while(done < filesize)
	{
		if(done + blocksize > filesize) {
			blocksize = filesize - done;
		}
		readBytes = read(fileNumber, buffer, blocksize);
		if(readBytes <= 0)
			break;
		//zipWriteInFileInZip(fileToZip,buffer,blocksize);
		done += readBytes;
	}
	close(fileNumber);

	if (done != filesize)
	{
		return -3;
	}
	return 0;
}

void Zip::Close() {
	//zipClose(fileToZip,NULL);
}


UnZip::UnZip(const char * zipPath) {
	memset(&fileToUnzip, 0, sizeof(fileToUnzip));
	fileStatus = mz_zip_reader_init_file(&fileToUnzip, zipPath, 0);
}

UnZip::~UnZip() {
	Close();
}
 
void UnZip::Close() {
	mz_zip_reader_end(&fileToUnzip);
}

int UnZip::ExtractFile(const char * internalPath, const char * path) {
	if(!fileStatus) return -1;
	
	mz_uint fileIndex = mz_zip_reader_locate_file(&fileToUnzip, internalPath, nullptr, 0);
	if(fileIndex < 0 ) return -2;

	mz_zip_archive_file_stat fileInfo;

	if (!GetFileInfo(&fileInfo, fileIndex))
		return -3;
	
	std::string fullPath(path);
	printf("Extracting file %s to: %s\n",internalPath,fullPath.c_str());
	if(Extract(path,&fileInfo) < 0 )
		return -1;
	return 0;
	
}

int UnZip::ExtractDir(const char * internalDir,const char * externalDir) {
	int i = 0;
	/*for(;;) {
		int code;
		if(i == 0) {
			code = unzGoToFirstFile(fileToUnzip);
			i++;
		} else {
			code = unzGoToNextFile(fileToUnzip);
		}
		if(code == UNZ_END_OF_LIST_OF_FILE) {
			if(i > 1) {
				return 0;
			} else {
				return -1;
			}
		}
		unz_file_info_s fileInfo[1];
	
		if(!GetFileInfo(fileInfo)) return -1;//Error
		
		char filename[fileInfo->size_filename+1];
		if(!GetFullFileName(fileInfo, filename)) return -2;//Error
		filename[fileInfo->size_filename]='\0';

		string outputPath(filename);
		if(outputPath.find(internalDir,0) != 0) {
			continue;
		}
		
		outputPath.replace(0,strlen(internalDir),externalDir);
		if(fileInfo->uncompressed_size != 0 && fileInfo->compression_method != 0) {
			//file
			i++;
			//printf("Extracting %s to: %s\n",GetFullFileName(fileInfo).c_str(),outputPath.c_str());
			Extract(outputPath.c_str(),fileInfo);
		}
		
	}*/
}

int UnZip::ExtractAll(const char * dirToExtract) {
	int i = 0;
	/*ifor(;;) {
		int code;
		f(i == 0) {
			code = unzGoToFirstFile(fileToUnzip);
			i++;
		} else {
			code = unzGoToNextFile(fileToUnzip);
		}
		if(code == UNZ_END_OF_LIST_OF_FILE) return 0;

		unz_file_info_s fileInfo[1];
	
		if(!GetFileInfo(fileInfo)) return -1;//Error

		std::string fileName(dirToExtract);
		fileName += '/';
		char fn[fileInfo->size_filename+1];
		if(!GetFullFileName(fileInfo, fn)) return -2;//Error
		fn[fileInfo->size_filename] = '\0';
		fileName += fn;
		if(fileInfo->uncompressed_size != 0 && fileInfo->compression_method != 0) {
			//file
			//printf("Extracting %s to: %s\n",GetFullFileName(fileInfo).c_str(),fileName.c_str());
			Extract(fileName.c_str(),fileInfo);
		}
	}*/
}

int UnZip::Extract(const char * path, const mz_zip_archive_file_stat* fileInfo) {
	//check to make sure filepath or fileInfo isnt null
	if(path == NULL || fileInfo == NULL)
		return -1;
	
	char folderPath[strlen(path)+1];
	
	strcpy(folderPath,path);
	char * pos = strrchr(folderPath,'/');

	if(pos != NULL) {
		*pos = '\0';
		CreateSubfolder(folderPath);
		//printf("Creating Subfolder %s\n", folderPath);
	}

	FILE* fp = fopen(path, "w");

	if(fp == NULL) {
		return -4;		
	}

	u32 uncompressed_size =  (u32) fileInfo->m_uncomp_size;
	
	if(uncompressed_size < ONE_MB){
		char buffer[uncompressed_size];
		char file[uncompressed_size];
		if(!mz_zip_reader_extract_to_mem_no_alloc(&fileToUnzip, fileInfo->m_file_index, &file, uncompressed_size, 0, &buffer, uncompressed_size))
			return -1;
		if(fwrite(&file, 1, uncompressed_size, fp) != uncompressed_size) 
			return -1;
	}
	else {
		size_t blocksize = ONE_MB;
		char buffer[blocksize];
		size_t pSize = ONE_MB/4;
		char passBuffer[pSize];
		u32 done = 0;
		int writeBytes = 0;
		while(done < fileInfo->m_uncomp_size)
		{
			if(done + blocksize > fileInfo->m_uncomp_size) {
				blocksize = fileInfo->m_uncomp_size - done;
			}
			if(!mz_zip_reader_extract_to_mem_no_alloc(&fileToUnzip, fileInfo->m_file_index, &buffer, blocksize, 0, &passBuffer, pSize))
				return -1;
			writeBytes = fwrite(&buffer, 1, uncompressed_size, fp);
			done += writeBytes;
			if(done <= 0) {
				break;
			}
		}
		if (done != fileInfo->m_uncomp_size)
			return -4;
	}

	fflush(fp);
	fclose(fp);
	
	return 0;
}

std::string UnZip::GetFileName(mz_zip_archive_file_stat* fileInfo) {
	char fileName[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE+1];
	std::string path;
	if(!GetFullFileName(fileInfo, fileName)) return string();
	fileName[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE] = '\0';
	char * pos = strrchr(fileName, '/');
	if (pos != NULL) {
		pos++;
		path = pos;
	} else {
		path = fileName;
	}
	return path;
}

bool UnZip::GetFullFileName(mz_zip_archive_file_stat* fileInfo, char* filename) {
	if(filename==nullptr) return false;
	strncpy(filename, fileInfo->m_filename, MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE-1);
	return true;
}

bool UnZip::GetFileInfo(mz_zip_archive_file_stat* fileInfo, const mz_uint& fileIndex) {
	if(fileInfo == nullptr) return false;
	return mz_zip_reader_file_stat(&fileToUnzip, fileIndex, fileInfo);
}
