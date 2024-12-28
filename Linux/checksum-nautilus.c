/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * checksum-nautilus.c
 * Copyright (C) 2020 Wei Keting <wkt@wkt-thinkpad-x1-extreme>
 *
 * gtk-foobar is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gtk-foobar is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>

#if defined(NE_4) && NE_4

#include <nautilus-extension.h>

#else

#include <libnautilus-extension/nautilus-menu-provider.h>

#endif

#include <glib-object.h>

#include "utils.h"

G_BEGIN_DECLS

#define CHECKSUM_TYPE_NAUTILUS             (checksum_nautilus_get_type ())
#define CHECKSUM_NAUTILUS(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHECKSUM_TYPE_NAUTILUS, ChecksumNautilus))
#define CHECKSUM_NAUTILUS_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), CHECKSUM_TYPE_NAUTILUS, ChecksumNautilusClass))
#define CHECKSUM_IS_NAUTILUS(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHECKSUM_TYPE_NAUTILUS))
#define CHECKSUM_IS_NAUTILUS_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), CHECKSUM_TYPE_NAUTILUS))
#define CHECKSUM_NAUTILUS_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), CHECKSUM_TYPE_NAUTILUS, ChecksumNautilusClass))

typedef struct _ChecksumNautilusClass ChecksumNautilusClass;
typedef struct _ChecksumNautilus ChecksumNautilus;


struct _ChecksumNautilusClass
{
	GObjectClass parent_class;
};

struct _ChecksumNautilus
{
	GObject parent_instance;

};

GType checksum_nautilus_get_type (void) G_GNUC_CONST;

G_END_DECLS


static const gchar*
get_nautilus_menu_item_label()
{
	const gchar * const *langs = g_get_language_names ();
	for(gint i=0;langs[i];i++){
		if(g_str_has_prefix(langs[i],"zh_")){
			return "计算校验码";
		}
	}
	return "Calculate checksum";
}

static gboolean
check_nautilus_menu_item_enabled()
{
	const gchar *app_names[] = {QT_APPLICATION_NAME,QT_APPLICATION_NAME_LOWER,NULL};
	gboolean ret = FALSE;
	for(int i=0;;i++){
		const gchar *an = app_names[i];
		if(an == NULL)break;
		gchar *kf = g_strdup_printf("%s" G_DIR_SEPARATOR_S QT_ORGANIZATION_NAME G_DIR_SEPARATOR_S  "%s.conf",g_get_user_config_dir(),an);
		gboolean found = FALSE;
		if(g_file_test(kf,G_FILE_TEST_IS_REGULAR)){
			GKeyFile *gkf = g_key_file_new();
			g_key_file_load_from_file(gkf,kf,G_KEY_FILE_NONE,NULL);
			GError *error = NULL;
			gboolean v = g_key_file_get_boolean(gkf,"General",KEY_ENABLE_FILE_MENU_ITEM,&error);
			if(error == NULL)ret = v;
			if(error)g_error_free(error);
			g_key_file_free(gkf);
			found = TRUE;
		}
		g_free(kf);
		if(found)break;
	}
	return ret;
}

static gchar cmd_args[4096] = {};

static void
nautilus_menu_item_activate_cb (NautilusMenuItem *item,const gchar *files){
	gchar *command = g_strdup_printf("qtchecksum %s",files);
	//g_printerr("command: %s\n",command);
	g_spawn_command_line_async(command,NULL);
	g_free (command);
}

static GList *
checksum_nautilus_get_background_items (NautilusMenuProvider *provider,
#ifndef NE_4
                                        GtkWidget            *window,
#endif
                                        NautilusFileInfo     *file_info)
{
/*	ChecksumNautilus *nautilus = CHECKSUM_NAUTILUS (provider);
	GList *items = g_list_append(NULL,nautilus_menu_item_new("bg items","bg_items","bg tip","qtchecksum"));
	return items;
*/
	return NULL;
}

static GList *
checksum_nautilus_get_file_items (NautilusMenuProvider *provider,
#ifndef NE_4
                                  GtkWidget            *window,
#endif
                                  GList                *files)
{
	ChecksumNautilus *nautilus = CHECKSUM_NAUTILUS (provider);
	if(files == NULL || files->data == NULL || !check_nautilus_menu_item_enabled())return NULL;
	guint n_files = g_list_length (files);
	gchar *uri;
	gchar *path;
	gint n = 0;
	bzero(cmd_args,sizeof(cmd_args));
	for(guint i=0;i<n_files;i++){
		uri = nautilus_file_info_get_uri(NAUTILUS_FILE_INFO(g_list_nth_data (files,i)));
		if(i == 0){
			if(!g_str_has_prefix (uri,"file://")){
				g_free(uri);
				return NULL;
			}
		}
		path = g_filename_from_uri(uri,NULL,NULL);
		g_free(uri);
		n+=snprintf(cmd_args+n,sizeof(cmd_args)-n,"%s'%s'",i==0?"":" ",path);
		g_free(path);
	}
	NautilusMenuItem *item = nautilus_menu_item_new("qtchecksum",
	                                                get_nautilus_menu_item_label (),
	                                                get_nautilus_menu_item_label (),
	                                                "qtchecksum");
	g_signal_connect(item,"activate",G_CALLBACK(nautilus_menu_item_activate_cb),cmd_args);
	GList *items = g_list_append(NULL,item);
	return items;
}

static void
checksum_nautilus_menu_provider_iface_init (
#if defined(NE_4) && NE_4
	NautilusMenuProviderInterface
#else
	NautilusMenuProviderIface
#endif
	*iface)
{
  iface->get_background_items = checksum_nautilus_get_background_items;
  iface->get_file_items = checksum_nautilus_get_file_items;
}

G_DEFINE_DYNAMIC_TYPE_EXTENDED (ChecksumNautilus, checksum_nautilus, G_TYPE_OBJECT, 0,
                                G_IMPLEMENT_INTERFACE_DYNAMIC (NAUTILUS_TYPE_MENU_PROVIDER,
                                                               checksum_nautilus_menu_provider_iface_init))

static void
checksum_nautilus_init (ChecksumNautilus *checksum_nautilus)
{
}

static void
checksum_nautilus_class_finalize (ChecksumNautilusClass *object)
{
}

static void
checksum_nautilus_class_init (ChecksumNautilusClass *klass)
{
}


/* Nautilus extension */

static GType type_list[1];

#define EXPORT __attribute__((__visibility__("default"))) extern

EXPORT void
nautilus_module_initialize (GTypeModule *module)
{
	checksum_nautilus_register_type (module);
	type_list[0] = CHECKSUM_TYPE_NAUTILUS;
}

EXPORT void
nautilus_module_shutdown (void)
{
}

EXPORT void
nautilus_module_list_types (const GType **types,
                            int          *num_types)
{
  *types = type_list;
  *num_types = G_N_ELEMENTS (type_list);
}
