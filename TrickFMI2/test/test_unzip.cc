/*!
@file test_unzip.cc
@brief Simple program to test FMU management functions.

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ftw.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>

// Archive library includes.
#include <archive.h>
#include <archive_entry.h>

int remove_callback(
   const char        * fpath,
   const struct stat * sb,
         int           typeflag,
         struct FTW  * ftwbuf    )
{
   // Remove the file or directory.
   int rm_status = remove( fpath );

   // Report error i
   if ( rm_status ) { perror( fpath ); }

   return( rm_status );
}


int remove_unpack_dir( const char * path )
{
    return( nftw( path, remove_callback, 64, FTW_DEPTH | FTW_PHYS ) );
}


int main( int nargs, char ** args )
{
   char cwd[MAXPATHLEN];
   const char fmu_file[] = "trickBall.fmu";
   const char unpack_dir[] = "unpack";

   struct archive *fmu;
   struct archive *unpack;
   struct archive_entry *entry;
   const void * buff;
   size_t size;
   off_t  offset;
   int flags;
   int status;
   bool verbose_output = true;
   bool cleanup = false;

   struct stat unpack_dir_stat;

   /* Select which attributes we want to restore. */
   flags = ARCHIVE_EXTRACT_TIME;
   flags |= ARCHIVE_EXTRACT_PERM;
   flags |= ARCHIVE_EXTRACT_ACL;
   flags |= ARCHIVE_EXTRACT_FFLAGS;

   // Set up the FMU archive to read.
   fmu = archive_read_new();
   archive_read_support_filter_all( fmu );
   archive_read_support_format_all( fmu );

   // Set up how to write out the archive.
   unpack = archive_write_disk_new();
   archive_write_disk_set_options( unpack, flags );
   archive_write_disk_set_standard_lookup( unpack );

   // Open the FMU archive.
   status = archive_read_open_filename( fmu, fmu_file, 10240 );
   if (status != ARCHIVE_OK) {
      fprintf( stderr, "Error opening FMU file: %s\n", fmu_file );
      exit(1);
   }

   // Get current working directory.
   if ( getwd( cwd ) == NULL ){
      fprintf( stderr, "Error getting current working directory: %s\n", cwd );
   }

   // Create the directory in which to unpack the archive.
   // First check to make sure that the directory doesn't already exist.
   if ( stat( unpack_dir, &unpack_dir_stat ) == 0 ){
      fprintf( stderr, "Unpack directory already exists: %s\n", unpack_dir );
      exit(1);
   }
   else {
      if ( errno != ENOENT ){
         perror( "Error associated with unpack directory" );
         exit(1);
      }
   }

   // Create the unpack directory.
   if ( mkdir( unpack_dir, 0000755 ) ) {
      perror( "Error creating the unpack directory" );
      exit(1);
   }

   // Move to the newly created directory.
   if ( chdir( unpack_dir ) ) {
      perror( "Error moving into the unpack directory" );
      exit(1);
   }

   // Read the next header in the FMU archive.
   status = archive_read_next_header( fmu, &entry );

   while (    (status != ARCHIVE_EOF)
           && ((status == ARCHIVE_OK) || (status == ARCHIVE_WARN)) ) {

      // Print out message if everything is not ARCHIVE_OK.
      if ( status < ARCHIVE_OK ) {
         fprintf( stderr, "%s\n", archive_error_string( fmu ) );
      }

      status = archive_write_header( unpack, entry );

      if ( status < ARCHIVE_OK ){
         fprintf( stderr, "%s\n", archive_error_string( unpack ) );
      }
      else if ( archive_entry_size( entry ) > 0 ) {

         // Read the next data block.
         status = archive_read_data_block( fmu, &buff, &size, &offset );
         if ( status < ARCHIVE_OK ) {
            fprintf( stderr, "%s\n", archive_error_string(unpack) );
         }

         // Loop through the data blocks.
         while (    (status != ARCHIVE_EOF)
                 && ((status == ARCHIVE_OK) || (status == ARCHIVE_WARN)) ) {

            // Write the data block.
            status = archive_write_data_block( unpack, buff, size, offset );
            if ( status < ARCHIVE_OK ) {
               fprintf( stderr, "%s\n", archive_error_string(unpack) );
            }
            if ( status < ARCHIVE_WARN ) {
               continue;
            }

            // Read the next data block.
            status = archive_read_data_block( fmu, &buff, &size, &offset );
         }

         // If ARCHIVE_EOF of archive entry then mark as ARCHIVE_OK.
         if ( status == ARCHIVE_EOF ){ status = ARCHIVE_OK; }
         if ( status < ARCHIVE_OK ) {
            fprintf( stderr, "%s\n", archive_error_string( unpack ) );
         }

      }
      status = archive_write_finish_entry( unpack );
      if ( status < ARCHIVE_OK ) {
         fprintf( stderr, "%s\n", archive_error_string( unpack ) );
      }

      // Print out progress if verbose setting.
      if ( verbose_output ) {
         printf("%s\n",archive_entry_pathname(entry));
      }

      // Read the next header in the FMU archive.
      status = archive_read_next_header( fmu, &entry );

   } // End of while loop.

   // Check for error.
   if ( status < ARCHIVE_WARN ) {
      fprintf( stderr, "%s\n", archive_error_string( fmu ) );
   }

   // Close the archive unpack object.
   archive_write_close( unpack );
   archive_write_free( unpack );

   // Check for error.
   if ( status < ARCHIVE_WARN ) {
      fprintf( stderr, "%s\n", archive_error_string( fmu ) );
   }

   // Close the archive file.
   archive_read_close(fmu);
   archive_read_free(fmu);

   // Check for error.
   if ( status < ARCHIVE_WARN ) {
      fprintf( stderr, "%s\n", archive_error_string( fmu ) );
      exit(1);
   }

   // Move back into the original current working directory.
   if ( chdir( cwd ) ) {
      perror( "Error moving back to the original directory" );
      exit(1);
   }

   if ( cleanup ) {
      // Remove the unpack directory.
      if ( remove_unpack_dir( unpack_dir ) ) {
         perror( "Error removing the unpack directory" );
         exit(1);
      }
   }

   return( 0 );

}
