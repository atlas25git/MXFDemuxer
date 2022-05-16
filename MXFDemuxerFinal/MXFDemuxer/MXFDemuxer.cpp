#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include<map>
#define CREATE_LOG

using namespace std;

vector<uint8_t> MAGIC_KEY = { 6 ,14 ,43 ,52 ,1 ,2 ,1 ,1 ,13 ,1 ,3 ,1};

int main(int argc, char* argv[])
{
	if (argc <= 1)
	{
		cout << "CMD argument not set\n";
	}
	
	//Basically based on fixed 12 bytes header look for the possible matches,
	//thereafter go for the last 4 bytes and use them as track ids to dump the data.

	string fileName = argv[1];

#ifdef CREATE_LOG
	string logFileName = fileName + ".txt";
	ofstream fout(logFileName);
	fout << fileName << "\n";
#endif

	map<uint32_t, ofstream> tracks;
	ifstream in(fileName,ios::binary);

	int counter = 0;
	int crcBytes = 0;
	//remove crc:run-in data
	while (in.peek() != 6)
	{
#ifdef CREATE_LOG
		fout << crcBytes++<<"\n";
#endif
		in.ignore(1);
	}
	
	while (!in.eof())
	{
		uint8_t* keyExtracted = new uint8_t[12];
		in.read(reinterpret_cast<char*>(keyExtracted), 12);
	
		bool flag = true;
		for (int i = 0; i < 12; i++)
		{
			if (MAGIC_KEY[i] != keyExtracted[i])
			{
				flag = false;
				break;
			}
		}
	

		if (flag)
		{
			counter++;
			uint32_t trackID;
			in.read(reinterpret_cast<char*>(&trackID), 4);
				uint8_t a, b, c, d = 0;
				

				in.read(reinterpret_cast<char*>(&d), 1);
				in.read(reinterpret_cast<char*>(&c), 1);
				in.read(reinterpret_cast<char*>(&b), 1);
				in.read(reinterpret_cast<char*>(&a), 1);

				d = 0;
				uintmax_t dataLen = (a & 255) + ((b & 255) << 8) + ((c & 255) << 16);
#ifdef CREATE_LOG
				fout << "###############TRACK BLOCK####################\n";
				fout << "TrackID: " << trackID << "\n";
				fout << (int)c << " " << (int)b << " " << (int)a << "\n";
				fout << dataLen << "\n";
#endif
				char* data = new char[dataLen];
				in.read(data, dataLen);
				if (tracks.count(trackID))
						tracks[trackID].write(data,dataLen);
				else {
					string fileNameTrack = fileName + " TrackID_" + to_string(trackID);
					tracks[trackID] = ofstream(fileNameTrack, ios::binary);
					tracks[trackID].write(data, dataLen);
				}

		}
		else
		{
	
			in.ignore(4);
			uint8_t a=0,b=0,c=0,d=0;

			in.read(reinterpret_cast<char*>(&d), 1);
			in.read(reinterpret_cast<char*>(&c), 1);
			in.read(reinterpret_cast<char*>(&b), 1);
			in.read(reinterpret_cast<char*>(&a), 1);


			uintmax_t dataLen = (a & 255) + ((b & 255) << 8) + ((c & 255) << 16);
#ifdef CREATE_LOG
			fout << "Not a track block\n";
			fout << dataLen << "\n";
#endif
			in.ignore(dataLen);
		}
	}
#ifdef CREATE_LOG
	fout<< "TotalBlocks: " << counter << "\n";
#endif 

	fout.close();

	return 0;
}

