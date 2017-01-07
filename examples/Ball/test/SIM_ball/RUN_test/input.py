
# Include the data recording specification.
execfile("Modified_data/data_record.dr")

# Specify the desired integration method.
my_integ_loop.getIntegrator(trick.Runge_Kutta_2, 4)
#my_integ_loop.getIntegrator(trick.Euler, 4)

# Turn off jobs in the S_define.
#trick.exec_set_job_onoff( "ball.ball_force_field", 2, False )
#trick.exec_set_job_onoff( "ball.ball_state_deriv", 2, False )

# Set the freeze frame rate
trick.exec_set_freeze_frame(0.10)

# Set the stop time.
trick.exec_set_terminate_time(100.0)

