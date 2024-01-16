#!/usr/bin/python3
# @file find_flaws.py
# @brief This program applies a flawfinder program to project source code.
#
# This is a Python program used to check the project source code using the
# flawfinder utility. https://dwheeler.com/flawfinder/
#
# @revs_title
# @revs_begin
# @rev_entry{ Dan Dexter, NASA ER6, Valkyrie, June 2020, --, Initial implementation, based on check_code.py.}
# @rev_entry{ Edwin Z. Crues, NASA ER7, Valkyrie, November 2021, --, Initial creation based off of TrickHLA scripts.}
# @rev_entry{ James Gentile, NASA ER7, July 2023, --, Initial creation based off of Valkyrie scripts.}
# @revs_end
#
import time
import os
import subprocess
import argparse
import git
import yaml

#from valkyrie_environment import *
from ci_utils import *
#from valkyrie_models import *

# Establish globals and set defaults for CI variables.
project_repo = git.Repo('.', search_parent_directories=True)
project_home = git.Repo('.', search_parent_directories=True).working_tree_dir
project_scripts_dir = os.path.join(project_home, 'scripts')
project_maintenance_dir = os.path.join(project_home, 'maintenance')
flawfinder_config = os.path.join(project_home, '.static-code-analysis.yml')

# Main routine.
def main():

   # Initialize the lists that go into the flawfinder command argument list.
   flawfinder_args = []

   #
   # Read configuration .yml file
   #
   with open(flawfinder_config) as file:
       config_file = yaml.safe_load(file)

       # Build up list of whitelist models
       whitelist_models = []
       if config_file['flawfinder_whitelist']:
           for model in config_file['flawfinder_whitelist']:
              whitelist_models.append(model)

       # Build up list of configs for cppcheck
       if config_file['flawfinder_configs']:
           for config in config_file['flawfinder_configs']:
               flawfinder_args.append(config)
   
   # Check Python version.
   #if sys.version_info[0] < 3:
   #   raise Exception("Must be using Python 3")

   #
   # Setup command line argument parsing.
   #
   parser = argparse.ArgumentParser( prog = 'find_code_flaws', \
                                     formatter_class = argparse.RawDescriptionHelpFormatter, \
                                     description = 'Check the project source code using flawfinder.' )

   parser.add_argument( '-c', '--clean', \
                        help = 'Clean all generated files.', \
                        action = 'store_true', dest = 'clean' )
   parser.add_argument( '-m0', \
                        help = 'Maximum flaw risk checking. Check source with minimum risk level 0 for inclusion in hitlist.', \
                        action = 'store_true', dest = 'm0' )
   parser.add_argument( '-m1', \
                        help = 'Check source with minimum risk level 1 for inclusion in hitlist.', \
                        action = 'store_true', dest = 'm1' )
   parser.add_argument( '-m2', \
                        help = 'Check source with minimum risk level 2 for inclusion in hitlist.', \
                        action = 'store_true', dest = 'm2' )
   parser.add_argument( '-m3', \
                        help = 'Check source with minimum risk level 3 for inclusion in hitlist.', \
                        action = 'store_true', dest = 'm3' )
   parser.add_argument( '-m4', \
                        help = 'Check source with minimum risk level 4 for inclusion in hitlist.', \
                        action = 'store_true', dest = 'm4' )
   parser.add_argument( '-m5', \
                        help = 'Minimal flaw risk checking. Check source with minimum risk level 5 for inclusion in hitlist.', \
                        action = 'store_true', dest = 'm5' )
   parser.add_argument('-p', '--project_home', help='Path to project directory.')
   parser.add_argument('-t', '--test', help='Do not check just show what would be done.', \
                         action='store_true')
   parser.add_argument('-v', '--verbose', help='Generate verbose output.', \
                         action='store_true')

   # Parse the command line arguments.
   args = parser.parse_args()

   # Check to see if we can find and setup the environment.
   if not project_home:
      CIMessage.failure('Could not find the project home directory!')
   if not project_scripts_dir:
      CIMessage.failure('Could not find the project scripts directory!')
      
   #
   # Now set and check for the necessary project directories.
   #
   # Now let's check to make sure that the project mantanence directory exist.
   project_maintenance_dir = os.path.join(project_home, 'maintenance')
   if not os.path.isdir( project_maintenance_dir ):
      CIMessage.failure(  'Could not find the project mantanence directory: '
                              + project_maintenance_dir)
      
#FIXME   # Let's make sure that Valkyrie model directory exists before we move there.
#FIXME   project_models_dir = os.path.join(project_home, 'models')
#FIXME   if os.path.isdir(project_models_dir):
#FIXME      if args.verbose:
#FIXME         CIMessage.status(  'Moving to Valkyrie models directory: '
#FIXME                                + project_models_dir)
#FIXME   else:
#FIXME      CIMessage.failure(  'Could not find the Valykrie models directory!: '
#FIXME                              + project_models_dir)
#FIXME      
#FIXME   # Move into the Valkyrie models directory.
#FIXME   os.chdir(project_models_dir)
   
   # Build the flawfinder directory paths.
   flawfinder_dir_path = os.path.join(project_home, 'maintenance', 'flawfinder')
   flawfinder_config_dir = os.path.join(flawfinder_dir_path, 'config')
   flawfinder_work_dir = os.path.join(flawfinder_dir_path, 'work')

   # Now let's check to make sure that the Valkyrie flawfinder directories exist.
   if not os.path.isdir( flawfinder_dir_path ):
      CIMessage.failure( 'Could not find the Valkyrie flawfinder directory: ' + flawfinder_dir_path)
   if not os.path.isdir( flawfinder_config_dir ):
      CIMessage.failure( 'Could not find the Valkyrie flawfinder config directory: ' + flawfinder_config_dir)
   if not os.path.isdir( flawfinder_work_dir ):
      CIMessage.failure( 'Could not find the Valkyrie flawfinder work directory: ' + flawfinder_work_dir)
   
   # Check for the clean option.
   if args.clean:
      clean_directory(flawfinder_work_dir, {'.gitkeep'}, args.test, args.verbose)
      CIMessage.success( 'Cleaned up the Valkyrie flawfinder artifacts.' )
      return

   # Find the flawfinder command and get the flawfinder version number.
   flawfinder_cmd, flawfinder_ver = find_flawfinder( None, False )
   if flawfinder_cmd is None:
      CIMessage.failure( 'No flawfinder command found!' )

   #
   # Let's do some command line option sanity checks.
   #
   arg_error = False

   # User must specify one of -m0, -m1, -m2, -m3, -m4, -m5.
   required_arg_cnt = 0
   if args.m0:
      # -m0
      required_arg_cnt += 1
   if args.m1:
      # -m1
      required_arg_cnt += 1
   if args.m2:
      # -m2
      required_arg_cnt += 1
   if args.m3:
      # -m3
      required_arg_cnt += 1
   if args.m4:
      # -m4
      required_arg_cnt += 1
   if args.m5:
      # -m5
      required_arg_cnt += 1
   if required_arg_cnt == 0:
      arg_error = True
      CIMessage.warning( 'You must specify one of \'-m0\', \'-m1\', \'-m2\', \'-m4\', \'-m4\' or \'-m5\'!' )
   elif required_arg_cnt >= 2:
      arg_error = True
      CIMessage.warning( 'Only specify one of \'-m0\', \'-m1\', \'-m2\', \'-m4\', \'-m4\' or \'-m5\'!' )

   if arg_error:
      CIMessage.failure( 'Error detected in parsing command arguments!' )

   #
   # Now let's check for paths to commands and directories.
   #
   
   # Build the list of models
   project_models = []
   project_models_ignore = []
   project_files = project_repo.git.execute(['git', 'ls-tree', '-r', 'HEAD', '--name-only']).split()
   for file in project_files:
      if file.find("/src") > -1 and file.find("/data") < 0 and file.find("/maintenance") < 0 and file.find("/html") < 0 and file.find("/apps") < 0:
         model_dir = file.split("/src")[0].split("/include")[0]
         if model_dir not in project_models:
            project_models.append(model_dir)
      elif file.find("/data") > -1:
          project_models_ignore.append(os.path.dirname(file))

   if config_file.get('flawfinder_model_dirs'):
      for model_dir in config_file['flawfinder_model_dirs']:
          project_models.append(model_dir)

   # Remove models from check that have been marked to be skipped
   for model in whitelist_models:
       if model in project_models:
           project_models.remove(model)

   # Set the flawfinder arguments based on the type of code checking the user wants to do.
   if args.m0:
      flawfinder_args.append( '--minlevel=0' )
   elif args.m1:
      flawfinder_args.append( '--minlevel=1' )
   elif args.m2:
      flawfinder_args.append( '--minlevel=2' )
   elif args.m3:
      flawfinder_args.append( '--minlevel=3' )
   elif args.m4:
      flawfinder_args.append( '--minlevel=4' )
   elif args.m5:
      flawfinder_args.append( '--minlevel=5' )
   else:
      flawfinder_args.append( '--minlevel=1' )

   # Add remaining args.
   flawfinder_args.append( '--allowlink' )
   flawfinder_args.append( '--context' )
   flawfinder_args.append( '--html' )
   
   # Iterate through the models.
   for model in project_models:
      
      # Check for the existance of the working directory.
      model_work_dir = os.path.join(flawfinder_work_dir, model)
      if not os.path.isdir(model_work_dir):
         # The work directory does not exist.  So, create it.
         if args.verbose:
            CIMessage.status('Creating work directory: ' + model_work_dir)
         os.makedirs(model_work_dir)
      else:
         if args.verbose:
            CIMessage.status('Using work directory: ' + model_work_dir)
      
      # Form the flawfind output file.
      flawfinder_output_file = os.path.join(model_work_dir, 'flawfinder_output.html')
      flawfinder_error_file = os.path.join(model_work_dir, 'flawfinder_error.txt')

      # Form the flawfinder command with command-line arguments.
      shell_command = [ flawfinder_cmd ]
      shell_command.extend( flawfinder_args )
      shell_command.append( model )
      
      # Check for test and verbose.
      if args.test:
         CIMessage.status('Would execute flawfind command: ' + ' '.join(shell_command))
         continue
      elif args.verbose:
         CIMessage.status('Executing flawfind command: ' + ' '.join(shell_command))

      # Execute the flawfinder command.
      try:
         
         # Creat the flawfinder output and error files.
         output_file = open(flawfinder_output_file, 'w')
         error_file = open(flawfinder_error_file, 'w')

         # Spawn off the flawfinder process using Popen.
         flawfinder_proc = subprocess.Popen( shell_command,
                                             stdout=output_file, 
                                             stderr=error_file)
         stdout, stderr = flawfinder_proc.communicate()

         while flawfinder_proc.poll() is None:
            # Check process activity 10 times a second.
            time.sleep( 0.1 )
            
         # Close the process output and error files.
         output_file.close()
         error_file.close()

      except subprocess.CalledProcessError:
         CIMessage.error( subprocess.CalledProcessError.message )
         CIMessage.failure( '\'flawfinder\' command failed!' )
         output_file.close()
         error_file.close()
         
   # END: Iterate through the models.

   # Show a summary of the settings.
   CIMessage.status( '---------------------------------------------------------------------' )
   CIMessage.status( 'project_home   = ' + project_home )
   CIMessage.status( 'project_models = ' + ', '.join( project_models ) )
   CIMessage.status( 'whitelist_models = ' + ', '.join( whitelist_models ) )
   CIMessage.status( 'flawfinder_cmd  = ' + flawfinder_cmd )
   CIMessage.status( 'flawfinder_ver  = ' + flawfinder_ver )
   CIMessage.status( 'flawfinder_args = ' + ' '.join( flawfinder_args ) )
   CIMessage.status( '---------------------------------------------------------------------' )

   # Let the user know that we are done.
   CIMessage.success( 'Finished checking project source code.' )

   return


# Function to find the flawfinder command.
#
# This function searches common locations for the flawfinder command.
#
# @return flawfinder_command The path to the flawfinder command.
# @return flawfinder_version The version of flawfinder.
# @param  flawfinder_bin     The path to the flawfinder programs directory.
# @param  verbose            Flag to set if verbose outputs are on.
#
def find_flawfinder( flawfinder_bin, verbose = True ):

   # Initialize the flawfinder command path.
   flawfinder_command = None
   flawfinder_version = None

   # Check to see if the path to the flawfinder binaries are set.
   if flawfinder_bin:

      # Use the command line path to set the command.
      flawfinder_command = flawfinder_bin + '/flawfinder'

   # flawfinder_bin is not set so lets check other options.
   else:

      # Check to see if the $FLAWFINDER_HOME environment variable is defined and set.
      flawfinder_home = os.environ.get( 'FLAWFINDER_HOME' )
      if flawfinder_home:

         # Check to see if FLAWFINDER_HOME exists.
         if os.path.isdir( flawfinder_home ):

            # Form the flawfinder command based on FLAWFINDER_HOME.
            flawfinder_command = flawfinder_home + '/flawfinder'

            if verbose:
               # Let the user know that we have FLAWFINDER_HOME and where it is.
               CIMessage.status( 'FLAWFINDER_HOME: ' + flawfinder_home )

         else:
            CIMessage.failure( 'FLAWFINDER_HOME not found: ' + flawfinder_home )

      else:

         # FLAWFINDER_HOME is not set so look in the standard locations for flawfinder.
         if os.path.isfile( '/usr/bin/flawfinder' ):
            flawfinder_command = '/usr/bin/flawfinder'
         elif os.path.isfile( '/usr/local/bin/flawfinder' ):
            flawfinder_command = '/usr/local/bin/flawfinder'
         elif os.path.isfile( '/opt/homebrew/bin/flawfinder' ):
            flawfinder_command = '/opt/homebrew/bin/flawfinder'
         elif os.path.isfile( '/Library/Frameworks/Python.framework/Versions/Current/bin/flawfinder' ):
            flawfinder_command = '/Library/Frameworks/Python.framework/Versions/Current/bin/flawfinder'

   # We're finished hunting. Now let's check for the flawfinder command.
   if flawfinder_command is None:
      CIMessage.failure( 'Could not find the flawfinder command!' )
   else:
      if not os.path.isfile( flawfinder_command ):
         CIMessage.failure( 'Could not find the flawfinder command!: '\
                                  +flawfinder_command )
      else:
         if verbose:
            CIMessage.status( 'Using flawfinder command: ' + flawfinder_command )

   #
   # Now get the flawfinder version ID tag.
   #
   try:
      flawfinder_version = str( subprocess.check_output( [flawfinder_command, '--version'] ) ).strip()
   except subprocess.CalledProcessError:
      CIMessage.error( subprocess.CalledProcessError.message )
      CIMessage.failure( '\'flawfinder --version\' command failed!: '
                               +flawfinder_command )

   return flawfinder_command, flawfinder_version


#
# Call the main function.
#
main()
