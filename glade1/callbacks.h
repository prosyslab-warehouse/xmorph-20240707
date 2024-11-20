#include <gtk/gtk.h>


gboolean
on_window_main_delete                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_load_session_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_session_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_add_an_image_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_view_images1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_resulting_image_size_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_show_morph_factors_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_interpolate_meshes1_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_morph_images1_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_restore_morph_factors1_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_restore_start_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_restore_end_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_store_morph_factors_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_store_start_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_store_end_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_morph_sequence1_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_guide_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_generic_help_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_warp_help_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_morph_help_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_why_the_beep_1_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_loadimage_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_loadmesh_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_savemesh_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_optionmenu_editview_released        (GtkButton       *button,
                                        gpointer         user_data);

void
on_optionmenu_editview_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_settings_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_do_mixing_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_save_image_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_back_to_guide_activate              (GtkButton       *button,
                                        gpointer         user_data);

void
on_color_feature_clicked               (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_drawingarea_color_expose_event      (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

void
on_combo_entry_feature_activate        (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_combo_entry_feature_changed         (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_new_feature_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_rename_feature_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_pack_feature_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_unpack_feature_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_delete_feature_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_tool_edit_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_tool_move_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_tool_stretch_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_tool_select_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_tool_unselect_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_tool_assign_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_viewport3_realize                   (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_motion_notify_event                 (GtkWidget       *widget,
                                        GdkEventMotion  *event,
                                        gpointer         user_data);

gboolean
on_button_press_event                  (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_expose_event                        (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

void
on_drawingarea_realize                 (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_drawingarea_button_release_event    (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_drawingarea_configure_event         (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

void
on_spinbutton_reswidth_changed         (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinbutton_reswidth_draw            (GtkWidget       *widget,
                                        GdkRectangle    *area,
                                        gpointer         user_data);

void
on_spinbutton_resheight_changed        (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinbutton_resheight_draw           (GtkWidget       *widget,
                                        GdkRectangle    *area,
                                        gpointer         user_data);

void
on_double_size_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_halve_size_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_resulting_apply_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_ok_button1_realize                  (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_ok_button1_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_image_win_1_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_optionmenu_editview_pressed         (GtkButton       *button,
                                        gpointer         user_data);

void
on_do_warp_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_handlebox_factors_show              (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_hscale_mesh_button_release_event    (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_hscale_image_button_release_event   (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_spinbutton_image_changed            (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinbutton_mesh_changed             (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_handleboxsubimage_show              (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_reset_subimage_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_spinbuttonx_changed                 (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinbuttonx_draw                    (GtkWidget       *widget,
                                        GdkRectangle    *area,
                                        gpointer         user_data);

void
on_spinbuttony_changed                 (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinbuttony_draw                    (GtkWidget       *widget,
                                        GdkRectangle    *area,
                                        gpointer         user_data);

void
on_spinbuttonw_changed                 (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinbuttonw_draw                    (GtkWidget       *widget,
                                        GdkRectangle    *area,
                                        gpointer         user_data);

void
on_spinbuttonh_changed                 (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_spinbuttonh_draw                    (GtkWidget       *widget,
                                        GdkRectangle    *area,
                                        gpointer         user_data);

void
on_subimage_apply_clicked              (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_dialogwarning_delete_event          (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_labelwarning_show                   (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_labelwarning_realize                (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_window_warped_delete_event          (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_drawingarea_warped_expose_event     (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

gboolean
on_drawingarea_warped_configure_event  (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

void
on_unselect_point_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_select_point_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_assign_point_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_add_horizontal_line_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_add_vertical_line_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_del_horizontal_line_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_del_vertical_line_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_movie_replay_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_movie_ok_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_movie_help_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_guide_delete_event                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_guide_destroy                       (GtkObject       *object,
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
on_question_delete_event               (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_yes_clicked                         (GtkButton       *button,
                                        gpointer         user_data);

void
on_no_clicked                          (GtkButton       *button,
                                        gpointer         user_data);

void
on_ok_button_color_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_mag_zoom_activate                   (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_mag_track_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

gboolean
on_mag_drawingarea_expose_event        (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

gboolean
on_mag_drawingarea_configure_event     (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data);

void
on_mag_unrealize                       (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_mag_zoom_changed                    (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_show_mag_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mag_spinbutton_activate             (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_mag_spinbutton_changed              (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_mag_track_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_restore_equal_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_drawingarea_key_press_event         (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

gboolean
on_imageselection1_destroy_event       (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_adjust_all_meshes_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_wavelet_equalize_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_adjust_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_menu_smooth_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
