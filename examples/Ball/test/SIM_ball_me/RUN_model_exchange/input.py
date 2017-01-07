
execfile( "Modified_data/trick_fmu.dr" )

trickBallFMU.fmu_path = 'fmu/trickBall.fmu'
trickBallFMU.logging_on = 0

trickBallFMU.position[0] = 5.0
trickBallFMU.position[1] = 5.0

trick.stop( 100.0 )

