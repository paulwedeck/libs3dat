#ifdef __cplusplus
extern "C" {
#include <s3dat_ext.h>
}
#else
#include <s3dat_ext.h>
#endif

#include <dirent.h>

#include <iostream>
#include <QStandardItemModel>
#include <QFileSystemModel>
#include <QApplication>
#include <QPushButton>
#include <QSplitter>
#include <QTreeView>
#include <QLabel>
#include <QDebug>
#include <QHeaderView>

s3util_exception_t* ex;


std::vector<s3dat_t*> s3dat_handles;
QStandardItemModel model(0, 5);

QStandardItem* newItem(QString name) {
	QStandardItem* item = new QStandardItem(name);
	item->setEditable(false);
	return item;
}

QPixmap ref_to_pixmap(s3dat_ref_t* ref) {
	uint32_t width = s3dat_width(ref);
	uint32_t height = s3dat_height(ref);

	int s = 1;
	if(width < 200 && height < 200) s = 4;

	uint32_t bmp_width = width*s;
	uint32_t bmp_height = height*s;

	s3util_color_t* original_color = s3dat_bmpdata(ref);
	uint8_t data[bmp_height][bmp_width][4];
	for(uint32_t x = 0;x != width;x++) {
		for(uint32_t y = 0;y != height;y++) {
			for(uint32_t xoff = 0;xoff != s;xoff++) {
				for(uint32_t yoff = 0;yoff != s;yoff++) {
					data[y*s+xoff][x*s+yoff][0] = original_color[y*width+x].red;
					data[y*s+xoff][x*s+yoff][1] = original_color[y*width+x].green;
					data[y*s+xoff][x*s+yoff][2] = original_color[y*width+x].blue;
					data[y*s+xoff][x*s+yoff][3] = original_color[y*width+x].alpha;
				}
			}
		}
	}
	return QPixmap::fromImage(QImage((uint8_t*)data, bmp_width, bmp_height, QImage::Format_RGBA8888));
}

QList<QStandardItem*>* refItem(uint16_t i, s3dat_ref_t* ref) {
	QList<QStandardItem*>* props = new QList<QStandardItem*>();
	if(ref != NULL) {
		props->append(newItem(QString::number(i)));

		if(s3dat_is_bitmap(ref)) {
			props->append(newItem(QString::number(s3dat_width(ref))));
			props->append(newItem(QString::number(s3dat_height(ref))));
		} else {
			props->append(newItem(""));
			props->append(newItem(""));
		}

		if(s3dat_is_animation(ref)) {
			props->append(newItem(QString::number(s3dat_anilen(ref))));
		} else {
			props->append(newItem(""));
		}

		if(s3dat_is_string(ref)) {
			props->append(newItem(QString::fromUtf8(s3dat_strdata(ref))));
		} else {
			props->append(newItem(""));
		}
	}
	return props;
}

void fillComplexTree(QString name, s3dat_content_type type, s3dat_t* handle, QStandardItem* fileitem) {
	QStandardItem* root = newItem(name);

	uint16_t len = s3dat_indexlen(handle, type);
	for(uint16_t i = 0;i != len;i++) {
		uint32_t seqlen = s3dat_seqlen(handle, i, type);

		QStandardItem* seqroot = newItem(QString::number(i));

		for(uint32_t seq = 0;seq != seqlen;seq++) {
			s3dat_ref_t* ref = NULL;
			if(type == s3dat_settler) ref = s3dat_extract_settler(handle, i, seq, &ex);
			else if(type == s3dat_torso) ref = s3dat_extract_torso(handle, i, seq, &ex);
			else if(type == s3dat_shadow) ref = s3dat_extract_shadow(handle, i, seq, &ex);
			else if(type == s3dat_string) ref = s3dat_extract_string(handle, i, seq, &ex);
			s3util_catch_exception(&ex);

			if(ref != NULL) {
				seqroot->appendRow(*refItem(seq, ref));
				s3dat_unref(ref);
			} else {
				printf("%i:%i\n", i, seq);
			}
		}
		root->appendRow(seqroot);
	}

	fileitem->appendRow(root);
}

void fillSimpleTree(QString name, s3dat_content_type type, s3dat_t* handle, QStandardItem* fileitem) {
	QStandardItem* root = newItem(name);

	uint16_t len = s3dat_indexlen(handle, type);
	for(uint16_t i = 0;i != len;i++) {

		s3dat_ref_t* ref = NULL;
		if(type == s3dat_gui) ref = s3dat_extract_gui(handle, i, &ex);
		else if(type == s3dat_landscape) ref = s3dat_extract_landscape(handle, i, &ex);
		else if(type == s3dat_animation) ref = s3dat_extract_animation(handle, i, &ex);
		else if(type == s3dat_palette) ref = s3dat_extract_palette(handle, i, &ex);
		s3util_catch_exception(&ex);

		root->appendRow(*refItem(i, ref));
		if(ref != NULL) s3dat_unref(ref);
	}

	fileitem->appendRow(root);
}

class Counter : public QObject
{

public:
    Counter() { m_value = 0; }

    int value() const { return m_value; }

public slots:
    void setValue(int value);

signals:
    void valueChanged(int newValue);

private:
    int m_value;
};

class S3DATCallback : public QObject {
	public:
	S3DATCallback();

	public slots:
	void onselect(const QModelIndex& index);
};

S3DATCallback::S3DATCallback() {}

QLabel* preview;

void S3DATCallback::onselect(const QModelIndex& index) {
	QModelIndex flevel = index.parent();
	QModelIndex slevel = flevel.parent();
	QModelIndex tlevel = slevel.parent();

	s3dat_ref_t* ref = NULL;

	if(!flevel.isValid() && !index.child(0, 0).isValid()) {
		QStandardItem* file = model.item(index.row());

		s3dat_t* datfile = s3dat_handles.at(index.row());

		fillComplexTree("settlers", s3dat_settler, datfile, file);
		fillComplexTree("torsos", s3dat_torso, datfile, file);
		fillComplexTree("shadows", s3dat_shadow, datfile, file);
		fillComplexTree("strings", s3dat_string, datfile, file);
		fillSimpleTree("guis", s3dat_gui, datfile, file);
		fillSimpleTree("landscapes", s3dat_landscape, datfile, file);
		fillSimpleTree("animations", s3dat_animation, datfile, file);
		fillSimpleTree("palettes", s3dat_palette, datfile, file);
		std::cout << "main " << index.row() << std::endl;
		return;
	}

	if(!slevel.isValid()) return;
	if(tlevel.isValid()) {
		switch(slevel.row()) {
			case 0:
				ref = s3dat_extract_settler(s3dat_handles.at(tlevel.row()), flevel.row(), index.row(), &ex);
			break;
			case 1:
				ref = s3dat_extract_torso(s3dat_handles.at(tlevel.row()), flevel.row(), index.row(), &ex);
			break;
			case 2:
				ref = s3dat_extract_shadow(s3dat_handles.at(tlevel.row()), flevel.row(), index.row(), &ex);
			break;
			default:
			break;
		}
	} else {
		if(flevel.row()<=3) return;

		switch(flevel.row()) {
			case 4:
				ref = s3dat_extract_gui(s3dat_handles[slevel.row()], index.row(), &ex);
			break;
			case 5:
				ref = s3dat_extract_landscape(s3dat_handles[slevel.row()], index.row(), &ex);
			break;
			case 6:
				ref = s3dat_extract_animation(s3dat_handles[slevel.row()], index.row(), &ex);
			break;
			case 7:
				ref = s3dat_extract_palette(s3dat_handles[slevel.row()], index.row(), &ex);
			break;
		}
	}
	if(!s3util_catch_exception(&ex) || ref == NULL) return;


	if(s3dat_is_bitmap(ref)) {
		preview->setPixmap(ref_to_pixmap(ref));
	}

	s3dat_unref(ref);
}

int main(int argc, char** argv) {
	DIR* dir = opendir("GFX");

	if(dir == NULL) {
		std::cout << "GFX not found!" << std::endl;
		return 1;
	}

	QApplication app(argc, argv);

	QSplitter splitter;

	QTreeView treeview(&splitter);
	treeview.setSelectionBehavior(QAbstractItemView::SelectRows);

	treeview.setModel(&model);

	struct dirent* dirent;

	while((dirent = readdir(dir)) != NULL) {
		std::string name = "GFX/";
		name += dirent->d_name;
		std::cout << name << std::endl;
		if(strlen(dirent->d_name) == 25 && strncmp(dirent->d_name+1, "iedler3", 7) == 0) {

			s3dat_t* datfile = s3dat_new_malloc();
			s3dat_readfile_name(datfile, (char*)name.c_str(), &ex);
			s3util_catch_exception(&ex);

			s3dat_add_utf8_encoding(datfile, &ex);
			s3dat_add_cache(datfile, &ex);
			s3util_catch_exception(&ex);

			QString id;
			id.append(dirent->d_name[9]);
			id.append(dirent->d_name[10]);
			QStandardItem* file = newItem(id);
			model.appendRow(file);

			s3dat_handles.push_back(datfile);
		}
	}

	model.setHorizontalHeaderItem(0, new QStandardItem("id"));
	model.setHorizontalHeaderItem(1, new QStandardItem("width"));
	model.setHorizontalHeaderItem(2, new QStandardItem("height"));
	model.setHorizontalHeaderItem(3, new QStandardItem("length"));
	model.setHorizontalHeaderItem(4, new QStandardItem("string"));

	preview = new QLabel(&splitter);
	S3DATCallback* callback = new S3DATCallback();

	splitter.show();
	QObject::connect(treeview.selectionModel(), &QItemSelectionModel::currentChanged, callback, &S3DATCallback::onselect);

	int return_var = app.exec();
	std::for_each(s3dat_handles.begin(), s3dat_handles.end(), s3dat_delete);

	delete preview;
	return return_var;
}

