#include "Network.h"//�������
#include "Buffer.h"//���������
#include "Storage.h"//�洢���
#include "FileIO.h"//�ļ�IO���
#include "Streams.h"//�����
using namespace std;
//��������
int main()
{
	//����C���µ�WindowsĿ¼
	RbsLib::Storage::StorageFile storageFile("C:\\Windows");//�����ļ��洢����
	if (storageFile.GetFileType() != RbsLib::Storage::FileType::Dir)//�ж��Ƿ�ΪĿ¼
	{
		cout << "����Ŀ¼" << endl;
		return 1;
	}
	for (auto file : storageFile)
	{
		//����Ŀ¼�µ��ļ�
		cout << file.Path() << endl;//����ļ�·��
	}

	//��ȡC���µ�a.txt�ļ�
	RbsLib::Storage::StorageFile file("C:\\a.txt");//�����ļ��洢����
	if (file.IsExist() == false)//�ж��ļ��Ƿ����
	{
		cout << "�ļ�������" << endl;
		return 1;
	}
	RbsLib::Storage::FileIO::File fp(file.Path(), RbsLib::Storage::FileIO::OpenMode::Read);//�����򿪵��ļ�����
	RbsLib::Buffer buffer = fp.Read(1024);//���ļ���ȡ1024�ֽڲ��洢�ڻ�������
	cout << buffer.Data() << endl;//����������е����ݣ���Ҫ����ȷ���������е��ַ�����'\0'��β

	//����һ��TCP�ͻ��˲����ͻ������е�����
	RbsLib::Network::TCP::TCPClient client;//����TCP�ͻ���
	auto connection = client.Connect("127.0.0.1", 8080);//���ӵ�ָ���ķ�����
	connection.Send(buffer);//���ͻ������е�����

	//�����Ѵ򿪵Ķ�����ڶ�������ʱ�Զ��ر�
	//�������๦����鿴�ĵ�
	return 0;
}