############################################################################
# @class CIMessage
# @file ci_message.py
# @brief Defines the general CI Python messaging class.
#
# This is a Python module used to define a consistent message handling
# approach for the Python based scripts that are used to manage the NExSyS
# code base and repositories.
############################################################################
# @revs_title
# @revs_begin
# @rev_entry{ James Gentile, NASA ER7, July 2023, --, Initial creation based off of Valyrie scripts.}
# @revs_end
############################################################################
import sys
import os

class CIMessage():

   _ECHO_COLOR    = ''         # Use the system default text color.
   _COMMENT_COLOR = '\033[95m' # Bright magenta
   #_STATUS_COLOR = '\033[93m' # Bright yellow
   _STATUS_COLOR  = ''         # Use the system default text color.
   _SUCCESS_COLOR = '\033[32m' # Blue
   _WARNING_COLOR = '\033[94m' # Bright blue
   _ERROR_COLOR   = '\033[91m' # Bright red
   _FAILURE_COLOR = '\033[91m' # Bright red
   _END_COLOR     = '\033[0m'  # Reset

   _ECHO_TAG    = ''
   _COMMENT_TAG = 'COMMENT: '
   _STATUS_TAG  = 'STATUS: '
   _SUCCESS_TAG = 'SUCCESS: '
   _WARNING_TAG = 'WARNING: '
   _ERROR_TAG   = 'ERROR: '
   _FAILURE_TAG = os.linesep + 'FAILURE: '

   _EOL = os.linesep

   @staticmethod
   def echo( message ):
      print ( CIMessage._ECHO_COLOR + CIMessage._ECHO_TAG \
              + message + CIMessage._END_COLOR )

   @staticmethod
   def comment( message ):
      print ( CIMessage._COMMENT_COLOR + CIMessage._COMMENT_TAG \
              + message + CIMessage._END_COLOR )

   @staticmethod
   def status( message ):
      print ( CIMessage._STATUS_COLOR + CIMessage._STATUS_TAG \
              + message + CIMessage._END_COLOR )

   @staticmethod
   def success( message ):
      print ( CIMessage._SUCCESS_COLOR + CIMessage._SUCCESS_TAG \
              + message + CIMessage._END_COLOR )

   @staticmethod
   def warning( message ):
      print ( CIMessage._WARNING_COLOR + CIMessage._WARNING_TAG \
              + message + CIMessage._END_COLOR )

   @staticmethod
   def error( message ):
      print ( CIMessage._ERROR_COLOR + CIMessage._ERROR_TAG \
              + message + CIMessage._END_COLOR )

   @staticmethod
   def failure( message ):
      sys.exit( CIMessage._FAILURE_COLOR + CIMessage._FAILURE_TAG \
                + message + CIMessage._END_COLOR + CIMessage._EOL )
