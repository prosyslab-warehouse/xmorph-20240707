
void guide_initialize();

gboolean guide_callback(char *what,
		    int imagenum);



/*********** callbacks *******************/
// FIXME there is also a call from buttons
//void
//on_back_to_guide_activate              (GtkMenuItem     *menuitem,
//                                        gpointer         user_data);

void
on_guide_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_guide_text_expose_event             (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

void
on_guide_prev_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_guide_do_it_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_guide_next_clicked                  (GtkButton       *button,
                                        gpointer         user_data);


gboolean
on_guide_delete_event                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);
//gboolean
//on_guide_destroy                  (GtkWidget       *widget,
///				   gpointer         user_data);

void
on_guide_destroy                       (GtkObject       *object,
                                        gpointer         user_data);


