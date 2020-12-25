#include <gtk/gtk.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include<algorithm> 

using namespace std;

string fname; //stores filename
int orig_width, orig_height;
GtkWidget* window;
int width, height, percent, size_kB;

// returns size of given file
float get_size(string fname){
    FILE* fp = fopen(fname.c_str(), "r");
    if (fp == NULL){
        perror("Error in opening file");
        exit(EXIT_FAILURE);
    }
    fseek(fp, 0L, SEEK_END);
    float sz = ftell(fp) / 1000.0;
    fseek(fp, 0L, SEEK_SET);
    fclose(fp);

    return sz;
}

// returns resolution of given image
string get_pixels(string fname){
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "identify -format '%%w %%h' '%s'", fname.c_str());
    FILE* fp = popen(buffer, "r");

    fscanf(fp, "%d %d", &orig_width, &orig_height);
    pclose(fp);

    string pixels = to_string(orig_width) + " X " + to_string(orig_height);
    return pixels;
}

string get_last_name(string fname){
    return fname.substr(fname.find_last_of('/') + 1);
}

// compute width using binary search for a given size
int get_ideal_width(string fname, string dest){
    int l = 1, r = orig_width;
    int m;
    char buffer[1024];
    while (l < r){
        m = l + (r - l) / 2;

        snprintf(buffer, sizeof(buffer), "convert '%s' -resize %d '%s'", fname.c_str(), m, dest.c_str());
        system(buffer);

        float size = get_size(dest);
        if (size == size_kB){
            return m;
        }
        else if (size < size_kB){
            l = m;
            if (r - l == 1)
                return m;
        }
        else{
            r = m - 1;
        }
    }
    return m;
}


static void spin_clicked1(GtkSpinButton* spinbutton, gpointer user_data){
    gint value = gtk_spin_button_get_value_as_int(spinbutton);
    width = value;
}
static void spin_clicked2(GtkSpinButton* spinbutton, gpointer user_data){
    gint value = gtk_spin_button_get_value_as_int(spinbutton);
    height = value;
}
static void spin_clicked3(GtkSpinButton* spinbutton, gpointer user_data){
    gint value = gtk_spin_button_get_value_as_int(spinbutton);
    percent = value;
}
static void spin_clicked4(GtkSpinButton* spinbutton, gpointer user_data){
    gint value = gtk_spin_button_get_value_as_int(spinbutton);
    size_kB = value;
}

static void open_dialog(GtkWidget* button, gpointer label){
    GtkWidget* dialog;
    dialog = gtk_file_chooser_dialog_new("Choose a Image file", GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, "Open", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_widget_show_all(dialog);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), g_get_current_dir());
    gint resp = gtk_dialog_run(GTK_DIALOG(dialog));

    if (resp == GTK_RESPONSE_OK){
        fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

        string exten = get_last_name(fname).substr(get_last_name(fname).find_last_of('.') + 1);
        transform(exten.begin(), exten.end(), exten.begin(), ::tolower);
        if (exten == "jpg" || exten == "jpeg" || exten == "png"){
            int sz = get_size(fname);
            string pixels = get_pixels(fname);
            string output = "Image Name: " + get_last_name(fname) + "\nSize: " + to_string(sz) + " kB\nPixels: " + pixels;

            gtk_label_set_text(GTK_LABEL(label), output.c_str());
        }
        else{
            gtk_label_set_markup(GTK_LABEL(label), "<span background=\"Red\">Error:</span>  Image not found");
        }

    }
    else{
        gtk_label_set_markup(GTK_LABEL(label), "<span background=\"Red\">You closed it </span>\nBrowse Again");
    }

    gtk_widget_destroy(dialog);
}

// do the compression of image
static void process(GtkWidget* button, gpointer window, gpointer label){

    if (fname == ""){
        string error = "<span background=\"Red\">Select a file </span>\n";
        gtk_label_set_markup(GTK_LABEL(label), error.c_str());
        return;
    }
    string fname_last = get_last_name(fname);
    string lastname = fname_last.substr(0, fname_last.find_last_of('.')) + "_converted." + fname_last.substr(fname_last.find_last_of('.') + 1);

    string dest = fname.substr(0, fname.find_last_of('/')) + "/" + lastname;

    char buffer[1024];

    if (width != 0 and height != 0)
        snprintf(buffer, sizeof(buffer), "convert '%s' -resize %dX%d! '%s'", fname.c_str(), width, height, dest.c_str());
    else if (width != 0)
        snprintf(buffer, sizeof(buffer), "convert '%s' -resize %d '%s'", fname.c_str(), width, dest.c_str());
    else if (height != 0)
        snprintf(buffer, sizeof(buffer), "convert '%s' -resize X%d '%s'", fname.c_str(), height, dest.c_str());
    else if (percent != 0)
        snprintf(buffer, sizeof(buffer), "convert '%s' -resize %d%% '%s'", fname.c_str(), percent, dest.c_str());
    else if (size_kB != 0){
        width = get_ideal_width(fname, dest);
        snprintf(buffer, sizeof(buffer), "convert '%s' -resize %d '%s'", fname.c_str(), width, dest.c_str());
    }
    else{
        string display = gtk_label_get_text(GTK_LABEL(label));

        string t = "<span background=\"Red\">Error: </span>";
        t = t + "Set at least one of width, height, percentage or size.\n" + display;
        gtk_label_set_markup(GTK_LABEL(label), t.c_str());
        return;
    }
    system(buffer);

    int sz = get_size(dest);
    string pixels = get_pixels(dest);
    string output = "<span background=\"Green\">SUCCESS!!!!</span>\nImage Name:" + lastname + "\nSize:" + to_string(sz) + " kB\nPixels:" + pixels;

    gtk_label_set_markup(GTK_LABEL(label), output.c_str());
}

static void reset(GtkWidget* button, gpointer window, gpointer label){
    gtk_label_set_text(GTK_LABEL(label), NULL);
    fname = "";
}

int main(int argc, char* argv[]){
    GtkWidget* button;
    GtkWidget* button1;
    GtkWidget* button2;
    GtkWidget* halign;
    GtkWidget* label;
    GtkWidget* fixed;
    GtkWidget* label1, * label2, * label3, * label4;
    GtkWidget* grid;
    GtkWidget* spin_button1, * spin_button2, * spin_button3, * spin_button4;
    GtkAdjustment* adjustment1, * adjustment2, * adjustment3, * adjustment4;
    gtk_init(&argc, &argv);
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Image Compressor");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(window), false);
    fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(window), fixed);
    button = gtk_button_new_with_label("Browse file");
    gtk_fixed_put(GTK_FIXED(fixed), button, 200, 235);
    gtk_widget_set_size_request(button, 80, 30);


    button1 = gtk_button_new_with_label("Convert");
    gtk_fixed_put(GTK_FIXED(fixed), button1, 357, 235);
    gtk_widget_set_size_request(button1, 80, 30);

    button2 = gtk_button_new_with_label("Reset");
    gtk_fixed_put(GTK_FIXED(fixed), button2, 500, 235);
    gtk_widget_set_size_request(button2, 80, 30);

    label = gtk_label_new("");
    gtk_fixed_put(GTK_FIXED(fixed), label, 325, 150);


    label1 = gtk_label_new("Width");
    adjustment1 = gtk_adjustment_new(0, 0, 5000, 1, 0, 0);
    spin_button1 = gtk_spin_button_new(adjustment1, 1, 0);

    label2 = gtk_label_new("Height");
    adjustment2 = gtk_adjustment_new(0, 0, 5000, 1, 0, 0);
    spin_button2 = gtk_spin_button_new(adjustment2, 1, 0);

    label3 = gtk_label_new("Percentage");
    adjustment3 = gtk_adjustment_new(0, 0, 100, 1, 0, 0);
    spin_button3 = gtk_spin_button_new(adjustment3, 1, 0);

    label4 = gtk_label_new("Size(kB)");
    adjustment4 = gtk_adjustment_new(0, 0, 10000, 1, 0, 0);
    spin_button4 = gtk_spin_button_new(adjustment4, 1, 0);

    gtk_widget_set_hexpand(spin_button1, TRUE);
    gtk_widget_set_hexpand(spin_button2, TRUE);
    gtk_widget_set_hexpand(spin_button3, TRUE);
    gtk_widget_set_hexpand(spin_button4, TRUE);


    g_signal_connect(spin_button1,
        "value-changed",
        G_CALLBACK(spin_clicked1),
        label1);
    g_signal_connect(spin_button2,
        "value-changed",
        G_CALLBACK(spin_clicked2),
        label2);
    g_signal_connect(spin_button3,
        "value-changed",
        G_CALLBACK(spin_clicked3),
        label3);
    g_signal_connect(spin_button4,
        "value-changed",
        G_CALLBACK(spin_clicked4),
        label4);


    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_attach(GTK_GRID(grid), spin_button1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), spin_button2, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 5, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), spin_button3, 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 3, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), spin_button4, 5, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 5, 1, 1, 1);

    gtk_fixed_put(GTK_FIXED(fixed), grid, 10, 20);


    g_signal_connect(button, "clicked", G_CALLBACK(open_dialog), label);
    g_signal_connect(button1, "clicked", G_CALLBACK(process), label);
    g_signal_connect(button2, "clicked", G_CALLBACK(reset), label);
    gtk_widget_show_all(window);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_main();
    return 0;
}