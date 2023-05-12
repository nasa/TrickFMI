
f = "Modified_data/trick_fmu.dr"
exec(compile(open(f, 'rb').read(), f, 'exec'), globals(), locals())

trickBallFMU.fmu_path = 'fmu/trickBall.fmu'

trickBallFMU.position[0] = 5.0;
trickBallFMU.position[1] = 5.0;

trick.stop( 100.0 )

