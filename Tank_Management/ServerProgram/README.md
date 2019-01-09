This is the server-side prgram.
A client will send the data of tanks which will be stored in the mysql database.
I created a mysql database on AWS RDS, so that this data can be analysed by the AWS tools provided.
To compile the program, type : gcc -o <objectfile_name> $(mysql_config --cflags) <progam_name>.c $(mysql_config --libs)
To run, type : ./<objectfile_name> <portno.>
