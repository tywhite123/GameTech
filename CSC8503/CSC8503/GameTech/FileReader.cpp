#include "FileReader.h"



FileReader::FileReader(string filename)
{
	 ifstream file("..\\..\\Assets\\Data" + filename);

	 if (file.is_open())
	 {
		 char in;
		 file.get(in);

		 string line;
		 while (getline(file, line)) {

			 if (in == 'x')
			 {
				 //BOX
			 }
			 else if (in == '.')
			 {
				 //NOWT
			 }
			 else if (in == 'S')
			 {
				 //BALL START POS;
			 }
			 else if (in == 'E')
			 {
				 //END GOAL
			 }
		 }
	 }
	 file.close();

}

FileReader::~FileReader()
{
}
