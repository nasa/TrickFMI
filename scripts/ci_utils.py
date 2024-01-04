############################################################################
# @file ci_utils.py
# @brief Defines a collection of useful functions.
#
# This Python module provides a collection of useful functions that are
# used across CI scripts.
############################################################################
# @revs_title
# @revs_begin
# @rev_entry{ Edwin Z. Crues, NASA ER7, CI, July 2023, --, Initial creation based off of CI scripts.}
# @revs_end
############################################################################

import os
import re
import subprocess

from ci_message import *


# Establish global Trick variables.
trick_home = ''
trick_ver = ''
trick_ver_year = ''

# Establish global JEOD variables.
jeod_home = ''


# Function to get the GIT repository version
#
# This routine will get the GIT hask for the current directory.
# @return status  GIT command return status. Returns True on success.
# @return hash    GIT hash of current repo. Returns None on error.
# @param  path    Path to repo to query.
# @param  verbose Flag for verbose output.
#
def get_git_version(path=None, verbose=False):
   
   original_dir = None
   
   # Check to see if a path was passed in.
   if path:
      if not os.path.isdir(path):
         if verbose:
            CIMessage.warning('Could not find GIT directory:' + path)
         return False, None

      # Move into the directory.
      if verbose:
         CIMessage.status( 'Moving to GIT directory: ' + path )
      original_dir = os.path.abspath(os.path.curdir)
      os.chdir(path)
   
   # Find the GIT command.
   git_command, git_version = find_git()
   if not git_command:
      if verbose: CIMessage.error('GIT command not found!')
      # Move back to the original directory.
      if original_dir: os.chdir(original_dir)
      # Return error
      return False, None
   
   # Get the current repository revision number.
   try:
      git_hash = subprocess.check_output([git_command, 'rev-parse', '--short', 'HEAD'],
                                         stderr=subprocess.STDOUT).strip()
      if verbose:
         CIMessage.status( "GIT Rev: " + git_hash )
   except subprocess.CalledProcessError as e:
      CIMessage.error('GIT revision command failed: ' + e.output.strip())
      # Return error
      return False, None

   # Move back to the original directory.
   if original_dir: os.chdir(original_dir)
   
   return True, git_hash


# Function to get the number of CPUs on the system
#
# This routine will query the system to determine the number of CPUs
# available for processing.
# @return num_cpus Number of CPUs on the system.
#
def get_number_of_cpus():

   # There has to be at least 1 CPU.
   num_cpus = 1

   if os.path.isfile( '/usr/bin/nproc' ):

      # Linux OS
      num_cpus = int( subprocess.check_output( ['/usr/bin/nproc', '--all'] ).strip() )

   elif os.path.isfile( '/proc/cpuinfo' ):

      # Linux OS
      num_cpus = int( subprocess.check_output( ['grep', '-c', '^processor', '/proc/cpuinfo'] ).strip() )

   elif os.path.isfile( '/usr/sbin/sysctl' ):

      # Check for Mac OS
      try:
         system_type = str( subprocess.check_output( ['/usr/bin/uname', '-s'] ) ).strip()
      except subprocess.CalledProcessError:
         CIMessage.error( subprocess.CalledProcessError.message )

      if 'Darwin' in system_type:
         num_cpus = int( subprocess.check_output( ['/usr/sbin/sysctl', '-n', 'hw.ncpu'] ).strip() )

   return num_cpus


# Function to recursively clean out a directory.
#
# This routine will recursively walk a directory and clean it out.
#
# @param dir     The directory to remove.
# @param verbose Flag to enable verbose output, default is False (no output).
#
def remove_directory(directory, verbose=False):
   
   # Does the directory exist?
   if os.path.isdir(directory):
      
      if verbose:
         CIMessage.status( 'Removing directory: ' + directory )
      
      # Walk the directory structure.
      for root, dirs, files in os.walk( directory ):
         
         # Remove all the files in the directory.
         # Otherwise, we will not be able to remove the directory.
         for file in files:
            if verbose:
               CIMessage.status( 'Removing file: ' + file )
            file_path = os.path.join( root, file )
            try:
               # Remove the file.
               os.remove( file_path )
            except Exception as e:
               CIMessage.error(str(e))
         
         # Remove all the directories in the directory.
         # Otherwise, we will not be able to remove this directory.
         for dir in dirs:
            dir_path = os.path.join( root, dir )
            try:
               remove_directory(dir_path, verbose)
            except Exception as e:
               CIMessage.error(str(e))
               
      # End walking the directory structure.
      
      # Now that the directory has been cleaned out, remove it.
      os.rmdir(directory)
               
   # End check if directory exists.
               
   return


# Function to clean out a directory.
#
# This routine will clean up all the files and directories in a directory
# except for any names that are matched in the exception set. The routine
# saves the current working directory and then moves to the specified
# directory if it exists.  If not, the routine returns with an error
# condition.
#
# @return status     Returns True if error detected.
# @param  dir_path   The path to the directory to clean out.
# @param  exceptions A list of file/directory patterns to ignore.
# @param  test_only  Flag indicating to only show what would be done.
# @param  verbose    Flag to set if verbose outputs are on.
#
def clean_directory(
   dir_path,
   exceptions = set(),
   test_only = False,
   verbose = True ):
   
   # Check to make sure the work directory exists.
   if not os.path.isdir(dir_path):
      if verbose:
         CIMessage.warning('Could not find directory:' + dir_path)
      return False
   
   # Check to see if this is only a test.
   if test_only:
      if verbose:
         CIMessage.warning('Would clean up directory:' + dir_path)
      return True

   # Move into the directory.
   if verbose:
      CIMessage.status( 'Cleaning up directory: ' + dir_path )
   original_dir = os.path.abspath(os.path.curdir)
   os.chdir(dir_path)
      
   # Iterate through the entries in the directory.
   for entry in os.listdir(dir_path):
         
      # Iterate through the exception set looking for a match.
      skip = False
      for expression in exceptions:
         regex = re.compile(expression)
         skip = (regex.search(entry) != None)
         if skip: break
      # Skip this entry if a match is found.   
      if skip: continue
         
      # Remove the file.
      if os.path.isfile(entry):
         if verbose:
            CIMessage.status( 'Removing file: ' + entry )
         file_path = os.path.join( dir_path, entry )
         try:
            os.remove( file_path )
         except Exception as e:
            CIMessage.error(str(e))
         
      # Or, remove the directory.
      elif os.path.isdir(entry):
         dir_path = os.path.join( dir_path, entry )
         try:
            remove_directory(entry, verbose)
         except Exception as e:
            CIMessage.error(str(e))
            
      # Or, something unknown.
      else:
         if verbose:
            CIMessage.status( 'Unknown entry: ' + entry )
               
   # End walking the directory structure.

   # Move back to the original directory.
   os.chdir(original_dir)

   return True


# Function to find and return the GIT command and version.
#
# This function searches common locations for the GIT command.
#
# @return git_command The path to the GIT command.
# @return git_version The version of GIT.
# @param  git         The path to the GIT program.
# @param  verbose     Flag to set if verbose outputs are on.
#
def find_git( git = None, verbose = False ):

   # Initialize the GIT command path.
   git_command = None
   git_version = None

   # Check to see if the path to the GIT command is set.
   if git:
      if os.path.isfile(git):
         git_command = git
      else:
         CIMessage.error('Could not find the GIT command: ' + git)
         return None, None

   # The GIT program path is not set; so, lets check other options.
   else:
      
      # See if the GIT command is in the command path.
      try:
         path = subprocess.check_output(['which', 'git']).strip()
         # Found GIT command in the command path.
         if os.access(path, os.X_OK):
            git_command = path
      # Not in the command path.  So, look in the standard locations for.
      except subprocess.CalledProcessError:
         if os.access('/usr/bin/git', os.X_OK):
            git_command = '/usr/bin/git'
         elif os.access('/usr/local/bin/git', os.X_OK):
            git_command = '/usr/local/bin/git'

   # We're finished hunting. Now let's check for an excutable GIT command.
   if git_command is None:
      CIMessage.error( 'Could not find the GIT command!' )
      return None, None
   elif not os.access(git_command, os.X_OK):
      CIMessage.error( 'GIT command is not executable: '\
                             + git_command )
      return None, None

   #
   # Now get the GIT version ID tag.
   #
   try:
      version_output = str( subprocess.check_output( [git_command, '--version'] ) ).strip()
      # Just take the first line.
      git_version_list = version_output.split(os.linesep)
      git_version = git_version_list[0]
   except subprocess.CalledProcessError:
      CIMessage.error( subprocess.CalledProcessError.message )
      CIMessage.failure('\'git --version\' command failed!: '+git_command)
      
   if verbose:
      CIMessage.status( "GIT command: " + git_command )
      CIMessage.status( "GIT version: " + git_version )

   return git_command, git_version


# Function to find the Trick home directory and return the Trick version.
#
# This function searches common locations for Trick and returns the version.
#
# @return trick_home          The location of to the Trick home directory.
# @return trick_version       The Trick version tag.
# @return trick_version_year  The Trick year version.
# @param trick_path           The path to the Trick home directory.
# @param verbose              Flag to set if verbose outputs are on.
#
def find_trick( trick_path = None, verbose = True ):

   # Initialize the Trick home directory path.
   trick_home = None

   trick_gte_cmd = ['trick-gte', 'TRICK_HOME']
   trick_version_cmd = None

   # Check to see if the path to the Trick home directory is already set.
   if trick_path:

      # Use the value passed in.
      trick_home = trick_path

   # The path to the Trick home directory was not passed in so lets check other options.
   else:

      # Check to see if the $TRICK_HOME environment variable is defined and set.
      trick_home = os.environ.get( 'TRICK_HOME' )
      if trick_home:

         # Check to see if TRICK_HOME exists.
         if os.path.isdir( trick_home ):
            if verbose:
               # Let the user know that we have CPPCHECK_HOME and where it is.
               CIMessage.status( 'TRICK_HOME: ' + trick_home )

         else:
            CIMessage.failure( 'TRICK_HOME not found: ' + trick_home )

      else:

         # Capture the output from the 'trick-gte TRICK_HOME' command.
         try:
            trick_home = str( subprocess.check_output( trick_gte_cmd ) ).strip()
         except subprocess.CalledProcessError:
            CIMessage.error( subprocess.CalledProcessError.message )
            CIMessage.failure( '\'trick-gte\' command failed!: ' + trick_gte_cmd )

         # Look in other possible locations.
         if trick_home == '':
            # TRICK_HOME is not set so look in the standard locations.
            if os.path.isdir( '/usr/local/Trick' ):
               trick_home = '/usr/local/Trick'
            elif os.path.isdir( '/Applications/Trick' ):
               trick_home = '/Applications/Trick'
            elif os.path.isdir( os.environ.get( 'HOME' ) + '/Trick' ):
               trick_home = os.environ.get( 'HOME' ) + '/Trick'

   # We're finished hunting. Now let's check for the Trick home directory.
   if trick_home is None:
      CIMessage.failure( 'Could not find the Trick home directory!' )
   else:
      if os.path.isdir( trick_home ) is False:
         CIMessage.failure( 'Could not find the Trick home directory: '\
                                  +trick_home )
      else:
         if verbose:
            CIMessage.status( 'Using TRICK_HOME: ' + trick_home )

   #
   # Now get the Trick version ID tag.
   #
   # Form the trick-version command based on TRICK_HOME.
   trick_version_cmd = os.path.join(trick_home, 'bin', 'trick-version')

   # Make sure that this is a file.
   if os.path.isfile( trick_version_cmd ) is False:
      CIMessage.failure( 'The \'trick-version\' command not found!: '\
                               +trick_version_cmd )

   # Capture the output from the trick-version command.
   try:
      trick_version = str( subprocess.check_output( trick_version_cmd ) ).strip()
   except subprocess.CalledProcessError:
      CIMessage.error( subprocess.CalledProcessError.message )
      CIMessage.failure( '\'trick-version -y\' command failed!: '\
                               +trick_version_cmd )

   # Capture the output from the trick-version -y command.
   trick_ver_yr_cmd = [trick_version_cmd, '-y']
   try:
      trick_version_year = str( subprocess.check_output( trick_ver_yr_cmd ) ).strip()
   except subprocess.CalledProcessError:
      CIMessage.error( subprocess.CalledProcessError.message )
      CIMessage.failure( '\'trick-version -y\' command failed!: '\
                               +trick_ver_yr_cmd )

   # Return what we found.
   return trick_home, trick_version, trick_version_year


# Function to find the JEOD home directory.
#
# This function searches common locations for the JEOD models.
#
# @return jeod_home  The location of to the JEOD home directory.
# @param  jeod_path  The path to the JEOD home directory.
# @param  verbose    Flag to set if verbose outputs are on.
#
def find_jeod_home( jeod_path = None, verbose = True ):

   # Initialize the JEOD home directory path.
   jeod_home = None

   # Check to see if the path to the JEOD home directory is already set.
   if jeod_path:

      # Use the value passed in.
      jeod_home = jeod_path

   # The path to the JEOD home directory was not passed in so lets check other options.
   else:

      # Check to see if the $JEOD_HOME environment variable is defined and set.
      jeod_home = os.environ.get( 'JEOD_HOME' )
      if jeod_home:

         # Check to see if JEOD_HOME exists.
         if os.path.isdir( jeod_home ):

            if verbose:
               # Let the use know that we have CPPCHECK_HOME and where it is.
               CIMessage.status( 'JEOD_HOME: ' + jeod_home )

         else:
            CIMessage.failure( 'JEOD_HOME not found: ' + jeod_home )

   # We're finished hunting. Now let's check for the JEOD home directory.
   if jeod_home is None:
      CIMessage.failure( 'Could not find the JEOD home directory!' )
   else:
      if os.path.isdir( jeod_home ) is False:
         CIMessage.failure( 'Could not find the JEOD home directory: ' + jeod_home )
      else:
         if verbose:
            CIMessage.status( 'Using JEOD_HOME: ' + jeod_home )

   # Return what we found.
   return jeod_home

