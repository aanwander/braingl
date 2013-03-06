/*
 * propertypath.h
 *
 *  Created on: Mar 6, 2013
 *      Author: Ralph Schurade
 */

#ifndef PROPERTYPATH_H_
#define PROPERTYPATH_H_

#include "property.h"

#include <QtCore/QDir>

class PathWidgetWithLabel;

class PropertyPath : public Property
{
    Q_OBJECT

public:
    PropertyPath( QString name, QDir value );
    virtual ~PropertyPath();

    QWidget* getWidget();

    QVariant getValue();

    void setValue( QVariant value );
    void setValue( QDir value );


private:
    QDir m_value;
    PathWidgetWithLabel* m_widget;


private slots:
    void widgetChanged( QDir value, int id );

signals:
    void valueChanged();
    void pathChanged( QDir value );
};

#endif /* PROPERTYPATH_H_ */
