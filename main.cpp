
//////////////////////////////////////////////////////////////////////
// LWO_reader : main.cpp
//
// app to test loader routines
//
// Ilkka Prusi 2007
//
//////////////////////////////////////////////////////////////////////

// 
#include "MemFile.h"
#include "LwoReader.h"

#include <iostream>

using namespace std;

int main( int argc, char *argv[] )
{
	if (argc < 2)
	{
		return EXIT_FAILURE;
	}

	// handler of file-IO
	// (add streaming if necessary in case of huge files)
	//
	CMemFile LwoFile(argv[1]);

	// handler of file-format into internal list
	//
	CLwoReader LwoReader;

	if (LwoFile.LoadFile() == false)
	{
		cout << "Failed to read file: " << LwoFile.GetFilename() << endl;
		return EXIT_FAILURE;
	}

	cout << "filesize: " << LwoFile.GetFilesize() << endl;
	cout << "filename: " << LwoFile.GetFilename() << endl;

	// pass buffer of file to Lwo-handler for parsing
	//
	if (LwoReader.ProcessFromFile(LwoFile) == false)
	{
		cout << "Failed to handle chunks from file: " << LwoFile.GetFilename() << endl;
		return EXIT_FAILURE;
	}

	// start using processed LWO-object from LwoReader..
	// get opengl-list from object (with conversion) and display

	// TODO: implement accessors such as:
	//LwoReader.GetGLVertices
	//LwoReader.GetGLNormals
	//LwoReader.GetDXVertices
	//LwoReader.GetDXNormals

	return EXIT_SUCCESS;
}

