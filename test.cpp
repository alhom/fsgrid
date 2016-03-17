#include <iostream>
#include "fsgrid.hpp"

int main(int argc, char** argv) {
   
   MPI_Init(&argc,&argv);

   int rank,size;
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   MPI_Comm_size(MPI_COMM_WORLD, &size);

   { // <-- scope to make sure grid destructor runs before finalize
      // Create a 8×8 Testgrid
      std::array<uint32_t, 3> globalSize{8,8,1};
      std::array<int, 3> isPeriodic{false,false,true};
      FsGrid<int,1> testGrid(globalSize, MPI_COMM_WORLD, isPeriodic);

      if(rank == 0) {
         std::cerr << " --- Test task mapping functions ---" << std::endl;
      }
      if(rank == 0) {
         for(int i=0; i<8; i++) {
            auto taskLid = testGrid.getTaskForGlobalID(8*1*0+8*0+i);
            std::cerr << "Cell ( " << i << ", 0, 0) is located on task "
               << taskLid.first << std::endl;
            std::cerr << "   and it has LocalID " << taskLid.second << std::endl;
         }
         for(int i=0; i<8; i++) {
            auto taskLid = testGrid.getTaskForGlobalID(8*1*0+8*i+0);
            std::cerr << "Cell ( " << 0 << ", " << i << ", 0) is located on task "
               << taskLid.first << std::endl;
            std::cerr << "   and it has LocalID " << taskLid.second << std::endl;
         }
         for(int i=0; i<8; i++) {
            auto taskLid = testGrid.getTaskForGlobalID(8*1*0+8*i+i);
            std::cerr << "Cell ( " << i << ", " << i << ", 0) is located on task "
               << taskLid.first << std::endl;
         }
      }

      if(rank == 0) {
         std::cerr << " --- Test data transfer into the grid ---" << std::endl;
      }
      // Fill in some junk data from task 0
      std::vector<int> fillData;
      if(rank == 0) {
         fillData.resize(8*8);

         // We are going to send data for all 8×8 Cells
         testGrid.setupForTransferIn(8*8);
         for(int y=0; y<8; y++) {
            for(int x=0; x<8; x++) {
               fillData[y*8 + x] = x*y;

               testGrid.setFieldData(y*8+x,fillData[y*8+x]);
            }
         }
      } else {
         // The others simply recieve
         testGrid.setupForTransferIn(0);
      }
      testGrid.finishTransfers();

      // Now have each task output their data
      for(int i=0; i<size; i++) {
         if(i == rank) {
            std::cerr << "Contents of Task #" << rank << ": " << std::endl;
            std::array<uint32_t,3> localSize = testGrid.getLocalSize();
            for(unsigned int y=0; y<localSize[1]; y++) {
               for(unsigned int x=0; x<localSize[0]; x++) {
                  std::cerr << testGrid.get(x,y,0) << ", ";
               }
               std::cerr << std::endl;
            }
         }
         MPI_Barrier(MPI_COMM_WORLD);
      }

   }
   MPI_Finalize();

   return 0;
}
