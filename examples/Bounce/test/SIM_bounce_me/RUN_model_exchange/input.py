
f = "Modified_data/data_record.dr"
exec(compile(open(f, 'rb').read(), f, 'exec'), globals(), locals())

ball.fmu_path = 'fmu/trickBounce.fmu'

# Configure the environment.
ball.gravity      = 9.81
ball.e            = 0.7
ball.floor        = 0.0

# Set the values for initial state.
ball.position     = 1.0
ball.velocity     = 0.0
ball.acceleration = -ball.gravity
ball.mass         = 1.0

# Set up control parameters for the Regula Falsi flor event.
ball.floor_event.fires     = 0
ball.floor_event.error_tol = 1.e-12
ball.floor_event.mode      = trick.Any

trick.stop( 2.5 )

