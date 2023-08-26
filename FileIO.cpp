#include "FileIO.h"
namespace fio = RbsLib::Storage::FileIO;
RbsLib::Storage::FileIO::File::File(void) noexcept:open_mode(RbsLib::Storage::FileIO::OpenMode::None){}

RbsLib::Storage::FileIO::File::File(const std::string& path, 
	RbsLib::Storage::FileIO::OpenMode open_mode,
	RbsLib::Storage::FileIO::SeekBase default_seek,
	int64_t default_offset)
{
	this->fp = nullptr;
	this->Open(path, open_mode, default_seek, default_offset);
}

RbsLib::Storage::FileIO::File::File(const RbsLib::Storage::FileIO::File& file)
{
	if (file.fp)
	{
		this->fp = file.fp;
		this->quote = file.quote;
		++*this->quote;
	}
	else
	{
		this->fp = nullptr;
		this->quote = nullptr;
	}
	this->open_mode = file.open_mode;
	
}

RbsLib::Storage::FileIO::File::File(RbsLib::Storage::FileIO::File&& file)
{
	this->fp = file.fp;
	this->open_mode = file.open_mode;
	this->quote = file.quote;
	file.fp = nullptr;
	file.quote = nullptr;
}

RbsLib::Storage::FileIO::File::~File() noexcept
{
	this->Close();
}

const RbsLib::Storage::FileIO::File& RbsLib::Storage::FileIO::File::operator=(const RbsLib::Storage::FileIO::File& file) noexcept
{
	if (&file == this) return *this;
	this->Close();
	if (this->fp = file.fp)
	{
		this->quote = file.quote;
		--*this->quote;
	}
	else
	{
		this->quote = nullptr;
	}
	return *this;
}

const RbsLib::Storage::FileIO::File& RbsLib::Storage::FileIO::File::operator=(RbsLib::Storage::FileIO::File&& file) noexcept
{
	if (&file == this) return *this;
	this->Close();
	this->open_mode = file.open_mode;
	this->fp = file.fp;
	this->quote = file.quote;
	file.quote = nullptr;
	file.fp = nullptr;
	return *this;
}

void RbsLib::Storage::FileIO::File::Open(const std::string& path,
	RbsLib::Storage::FileIO::OpenMode open_mode,
	RbsLib::Storage::FileIO::SeekBase default_seek,
	int64_t default_offset)
{
	if (this->fp) throw FileIOException("This object has already opened the file");
	const char* mode;
	if ((open_mode & fio::OpenMode::Read) == fio::OpenMode::Read)
		if ((open_mode & fio::OpenMode::Write) == fio::OpenMode::Write)
			if ((open_mode & fio::OpenMode::Replace) == fio::OpenMode::Replace)
				mode = "w+";
			else
				mode = "a+";
		else if ((open_mode & fio::OpenMode::Replace) == fio::OpenMode::Replace)
			throw FileIOException("Read only and replace is a meaningless combination method");
		else
			mode = "r";
	else if ((open_mode & fio::OpenMode::Write) == fio::OpenMode::Write)
		if ((open_mode & fio::OpenMode::Replace) == fio::OpenMode::Replace)
			mode = "w";
		else
			mode = "a";
	else if ((open_mode & fio::OpenMode::Replace) == fio::OpenMode::Replace)
		throw FileIOException("Replace only is a meaningless method");
	else
		throw FileIOException("Unknown open mode");
	std::string str = mode;
	if ((open_mode & fio::OpenMode::Bin) == fio::OpenMode::Bin)
		str += "b";
	this->fp = fopen(path.c_str(), str.c_str());
	if (!this->fp)
		throw FileIOException(std::string("File open failed:") + strerror(errno));
	try
	{
		int ret;
		switch (default_seek)
		{
		case RbsLib::Storage::FileIO::SeekBase::begin:
		case RbsLib::Storage::FileIO::SeekBase::end:
			ret = this->Seek(default_seek, default_offset);
			break;
		case RbsLib::Storage::FileIO::SeekBase::now:
		default:

			throw FileIOException("Unsupported seek");
		}
		if (ret)
		{
			throw FileIOException(std::string("Seek failed:") + strerror(errno));
		}
	}
	catch (...)
	{
		fclose(this->fp);
		this->fp = nullptr;
		throw;
	}
	this->open_mode = open_mode;
	this->quote = new int;
	*this->quote = 1;
}

void RbsLib::Storage::FileIO::File::Close()
{
	if (this->fp)
	{
		--*this->quote;
		if (!*this->quote)
		{
			delete this->quote;
			this->quote = nullptr;
			fclose(this->fp);
			this->fp = nullptr;
		}
		else
		{
			this->quote = nullptr;
			this->fp = nullptr;
		}
	}
}

RbsLib::Buffer RbsLib::Storage::FileIO::File::Read(int64_t size) const
{
	if (!this->fp) throw fio::FileIOException("File is not open");
	if ((this->open_mode & RbsLib::Storage::FileIO::OpenMode::Read) != RbsLib::Storage::FileIO::OpenMode::Read)
		throw fio::FileIOException("Not have read permission");
	char* arr = new char[size];
	int64_t s;
	if ((s=fread(arr,1,size,this->fp))<=0) 
		throw RbsLib::Storage::FileIO::FileIOException("Read file failed");
	RbsLib::Buffer buffer(arr,s);
	return buffer;
}

void RbsLib::Storage::FileIO::File::Write(const RbsLib::IBuffer& buffer) const
{
	if (!this->fp) throw fio::FileIOException("File is not open");
	if ((this->open_mode & RbsLib::Storage::FileIO::OpenMode::Write) != RbsLib::Storage::FileIO::OpenMode::Write)
		throw fio::FileIOException("Not have write permission");
	char* data = new char[buffer.GetLength()];
	if (1!=fwrite(buffer.Data(), buffer.GetLength(), 1,this->fp ))
		throw RbsLib::Storage::FileIO::FileIOException("Write file failed");
}

int RbsLib::Storage::FileIO::File::Seek(SeekBase base, int64_t offset) const
{
	if (!this->fp) throw fio::FileIOException("File is not open");
	switch (base)
	{
	case RbsLib::Storage::FileIO::SeekBase::begin:
		return fseek(this->fp, SEEK_SET, offset);
	case RbsLib::Storage::FileIO::SeekBase::end:
		return fseek(this->fp, SEEK_END, offset);
	case RbsLib::Storage::FileIO::SeekBase::now:
		return fseek(this->fp, SEEK_CUR, offset);
	}
	throw fio::FileIOException("Unknown seek base");
	return -1;
}

bool RbsLib::Storage::FileIO::File::IsOpen(void) const noexcept
{
	return (bool)this->fp;
}

fio::OpenMode RbsLib::Storage::FileIO::File::GetOpenMode(void) const noexcept
{
	return this->open_mode;
}

bool RbsLib::Storage::FileIO::File::CheckOpenMode(fio::OpenMode mode) const noexcept
{
	if ((this->open_mode & mode) == mode) return true;
	else return false;
}

bool RbsLib::Storage::FileIO::File::CheckEOF(void) const noexcept
{
	return feof(this->fp);
}

RbsLib::Storage::FileIO::OpenMode RbsLib::Storage::FileIO::operator|(RbsLib::Storage::FileIO::OpenMode l, RbsLib::Storage::FileIO::OpenMode r) noexcept
{
	return RbsLib::Storage::FileIO::OpenMode((int)l | (int)r);
}

RbsLib::Storage::FileIO::OpenMode RbsLib::Storage::FileIO::operator&(RbsLib::Storage::FileIO::OpenMode l, RbsLib::Storage::FileIO::OpenMode r) noexcept
{
	return RbsLib::Storage::FileIO::OpenMode((int)l & (int)r);
}
