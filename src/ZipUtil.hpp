#pragma once
	
// use included minizip library
//#include "zip.h"
//#include "unzip.h"
#define MINIZ_NO_MALLOC
#define MINIZ_NO_TIME
#define MINIZ_HEADER_FILE_ONLY
#include "miniz.c"

#include <string>

class Zip {
	public:		
		Zip(const char * zipPath);
		void Close();
		~Zip();
		int AddFile(const char * internalPath,const char * path);
		int AddDir(const char * internalDir,const char * externalDir);
	private:
		int Add(const char * path);
		mz_zip_archive fileToZip;
		mz_bool fileStatus;
};
class UnZip {
	public:		
		UnZip(const char * zipPath);
		~UnZip();
		void Close();
		int ExtractFile(const char * internalPath,const char * path);
		int ExtractAll(const char * dirToExtract);
		int ExtractDir(const char * internalDir,const char * externalDir);
	private:
		int Extract(const char * path, const mz_zip_archive_file_stat* fileInfo);
		std::string GetFileName(mz_zip_archive_file_stat* fileInfo);
		bool GetFullFileName(mz_zip_archive_file_stat* fileInfo, char* filename);
		bool GetFileInfo(mz_zip_archive_file_stat* fileInfo, const mz_uint& fileIndex);
		mz_zip_archive fileToUnzip;
		mz_bool fileStatus;
};

