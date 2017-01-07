
# This one of several ways to include files from the input file
execfile("Modified_data/data_record.dr")

#my_integ_loop.integ_sched.integ_ptr = trick.getIntegrator( trick.Runge_Kutta_2, 4, 1.0);
my_integ_loop.getIntegrator(trick.Runge_Kutta_2, 2)
#my_integ_loop.getIntegrator(trick.Euler, 2)

# Set the freeze frame rate
trick.exec_set_freeze_frame(0.10)

# Set the stop time
trick.exec_set_terminate_time(2.5)

