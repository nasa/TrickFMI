
f = "Modified_data/data_record.dr"
exec(compile(open(f, 'rb').read(), f, 'exec'), globals(), locals())

ball.fmu_path = 'fmu/trickBounce.fmu'

ball.position     =  1.0;
ball.velocity     =  0.0;
ball.acceleration = -9.81;
ball.mass         =  1.0;
ball.gravity      =  9.81;
ball.e            =  0.7;
ball.floor        =  0.0;

trick.stop( 2.5 )

