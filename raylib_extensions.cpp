#include "raylib_extensions.hpp"

// Draw a grid centered at (0, 0, 0)
void DrawGrid(int slices, float spacing, Vector3 center)
{
    int halfSlices = slices / 2;

    rlBegin(RL_LINES);
        for (int i = -halfSlices; i <= halfSlices; i++)
        {
            if (i == 0)
            {
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
            }
            else
            {
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
            }

            rlVertex3f(center.x +(float) i * spacing, center.y, center.z + (float)-halfSlices*spacing);
            rlVertex3f(center.x + (float) i * spacing, center.y, center.z +  (float)halfSlices*spacing);

            rlVertex3f(center.x +  (float)-halfSlices*spacing, center.y, center.z + (float)i*spacing);
            rlVertex3f(center.x + (float)halfSlices*spacing, center.y, center.z + (float)i*spacing);
        }
    rlEnd();
}

#include <stdlib.h>
#include <stdio.h>
#include "scratch.hpp"

unsigned char *LoadFileData_scratch(const char *fileName, int *dataSize)
{
    unsigned char *data = 0;
    *dataSize = 0;

    if (fileName)
    {
        FILE *file = fopen(fileName, "rb");

        if (file != NULL)
        {
         
            fseek(file, 0, SEEK_END);
            int size = ftell(file);    
            fseek(file, 0, SEEK_SET);

            if (size > 0)
            {
                data = (unsigned char *) scratch_alloc(size * sizeof(unsigned char));

                if (data != NULL)
                {
                    size_t count = fread(data, sizeof(unsigned char), size, file);

                    if (count > 2147483647)
                    {
                        TRACELOG(LOG_WARNING, "FILEIO: [%s] File is bigger than 2147483647 bytes, avoid using LoadFileData()", fileName);
                        data = NULL;
                    }
                    else
                    {
                        *dataSize = (int)count;
                        if ((*dataSize) != size) TRACELOG(LOG_WARNING, "FILEIO: [%s] File partially loaded (%i bytes out of %i)", fileName, dataSize, count);
                        else TRACELOG(LOG_INFO, "FILEIO: [%s] File loaded successfully", fileName);
                    }
                }
                else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to allocated memory for file reading", fileName);
            }
            else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to read file", fileName);
            fclose(file);
        }
        else TRACELOG(LOG_WARNING, "FILEIO: [%s] Failed to open file", fileName);
    }
    else TRACELOG(LOG_WARNING, "FILEIO: File name provided is not valid");

    return data;
}
