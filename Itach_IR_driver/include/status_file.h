
#ifndef STATUS_FILE_H
#define STATUS_FILE_H

bool status_file_is_empty();
void init_status_file();
void record_status(const char *sid, const int status_value);
int  get_status_form_file(const char *sid);


#endif	//STATUS_FILE_H
