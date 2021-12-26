/***************************************************************************
                    qgsoraclenewconnection.cpp  -  description
                             -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMessageBox>
#include <QInputDialog>
#include <QRegExpValidator>

#include "qgssettings.h"
#include "qgsoraclenewconnection.h"
#include "qgsdatasourceuri.h"
#include "qgsoracletablemodel.h"
#include "qgsoracleconnpool.h"

QgsOracleNewConnection::QgsOracleNewConnection( QWidget *parent, const QString &connName, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mOriginalConnName( connName )
{
  setupUi( this );

  txtSchema->setShowClearButton( true );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsOracleNewConnection::showHelp );
  connect( btnConnect, &QPushButton::clicked, this, &QgsOracleNewConnection::testConnection );

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    QgsSettings settings;

    QString key = QStringLiteral( "/Oracle/connections/" ) + connName;
    txtDatabase->setText( settings.value( key + QStringLiteral( "/database" ) ).toString() );
    txtHost->setText( settings.value( key + QStringLiteral( "/host" ) ).toString() );
    QString port = settings.value( key + QStringLiteral( "/port" ) ).toString();
    if ( port.length() == 0 )
    {
      port = QStringLiteral( "1521" );
    }
    txtPort->setText( port );
    txtOptions->setText( settings.value( key + QStringLiteral( "/dboptions" ) ).toString() );
    txtWorkspace->setText( settings.value( key + QStringLiteral( "/dbworkspace" ) ).toString() );
    txtSchema->setText( settings.value( key + QStringLiteral( "/schema" ) ).toString() );
    cb_userTablesOnly->setChecked( settings.value( key + QStringLiteral( "/userTablesOnly" ), false ).toBool() );
    cb_geometryColumnsOnly->setChecked( settings.value( key + QStringLiteral( "/geometryColumnsOnly" ), true ).toBool() );
    cb_allowGeometrylessTables->setChecked( settings.value( key + QStringLiteral( "/allowGeometrylessTables" ), false ).toBool() );
    cb_useEstimatedMetadata->setChecked( settings.value( key + QStringLiteral( "/estimatedMetadata" ), false ).toBool() );
    cb_onlyExistingTypes->setChecked( settings.value( key + QStringLiteral( "/onlyExistingTypes" ), true ).toBool() );
    cb_includeGeoAttributes->setChecked( settings.value( key + QStringLiteral( "/includeGeoAttributes" ), false ).toBool() );

    if ( settings.value( key + QStringLiteral( "/saveUsername" ) ).toString() == QLatin1String( "true" ) )
    {
      txtUsername->setText( settings.value( key + QStringLiteral( "/username" ) ).toString() );
      chkStoreUsername->setChecked( true );
    }

    if ( settings.value( key + QStringLiteral( "/savePassword" ) ).toString() == QLatin1String( "true" ) )
    {
      txtPassword->setText( settings.value( key + QStringLiteral( "/password" ) ).toString() );
      chkStorePassword->setChecked( true );
    }

    // Old save setting
    if ( settings.contains( key + QStringLiteral( "/save" ) ) )
    {
      txtUsername->setText( settings.value( key + QStringLiteral( "/username" ) ).toString() );
      chkStoreUsername->setChecked( !txtUsername->text().isEmpty() );

      if ( settings.value( key + QStringLiteral( "/save" ) ).toString() == QLatin1String( "true" ) )
        txtPassword->setText( settings.value( key + QStringLiteral( "/password" ) ).toString() );

      chkStorePassword->setChecked( true );
    }

    txtName->setText( connName );
  }
  txtName->setValidator( new QRegExpValidator( QRegExp( QStringLiteral( "[^\\/]+" ) ), txtName ) );
}

void QgsOracleNewConnection::accept()
{
  QgsSettings settings;
  QString baseKey = QStringLiteral( "/Oracle/connections/" );
  settings.setValue( baseKey + QStringLiteral( "selected" ), txtName->text() );

  if ( chkStorePassword->isChecked() &&
       QMessageBox::question( this,
                              tr( "Saving Passwords" ),
                              tr( "WARNING: You have opted to save your password. It will be stored in plain text in your project files and in your home directory on Unix-like systems, or in your user profile on Windows. If you do not want this to happen, please press the Cancel button.\n" ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // warn if entry was renamed to an existing connection
  if ( ( mOriginalConnName.isNull() || mOriginalConnName.compare( txtName->text(), Qt::CaseInsensitive ) != 0 ) &&
       ( settings.contains( baseKey + txtName->text() + QStringLiteral( "/service" ) ) ||
         settings.contains( baseKey + txtName->text() + QStringLiteral( "/host" ) ) ) &&
       QMessageBox::question( this,
                              tr( "Save Connection" ),
                              tr( "Should the existing connection %1 be overwritten?" ).arg( txtName->text() ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // on rename delete the original entry first
  if ( !mOriginalConnName.isNull() && mOriginalConnName != txtName->text() )
  {
    settings.remove( baseKey + mOriginalConnName );
    settings.sync();
  }

  baseKey += txtName->text();
  settings.setValue( baseKey + QStringLiteral( "/database" ), txtDatabase->text() );
  settings.setValue( baseKey + QStringLiteral( "/host" ), txtHost->text() );
  settings.setValue( baseKey + QStringLiteral( "/port" ), txtPort->text() );
  settings.setValue( baseKey + QStringLiteral( "/username" ), chkStoreUsername->isChecked() ? txtUsername->text() : QString() );
  settings.setValue( baseKey + QStringLiteral( "/password" ), chkStorePassword->isChecked() ? txtPassword->text() : QString() );
  settings.setValue( baseKey + QStringLiteral( "/userTablesOnly" ), cb_userTablesOnly->isChecked() );
  settings.setValue( baseKey + QStringLiteral( "/geometryColumnsOnly" ), cb_geometryColumnsOnly->isChecked() );
  settings.setValue( baseKey + QStringLiteral( "/allowGeometrylessTables" ), cb_allowGeometrylessTables->isChecked() );
  settings.setValue( baseKey + QStringLiteral( "/estimatedMetadata" ), cb_useEstimatedMetadata->isChecked() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
  settings.setValue( baseKey + QStringLiteral( "/onlyExistingTypes" ), cb_onlyExistingTypes->isChecked() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
  settings.setValue( baseKey + QStringLiteral( "/includeGeoAttributes" ), cb_includeGeoAttributes->isChecked() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
  settings.setValue( baseKey + QStringLiteral( "/saveUsername" ), chkStoreUsername->isChecked() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
  settings.setValue( baseKey + QStringLiteral( "/savePassword" ), chkStorePassword->isChecked() ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
  settings.setValue( baseKey + QStringLiteral( "/dboptions" ), txtOptions->text() );
  settings.setValue( baseKey + QStringLiteral( "/dbworkspace" ), txtWorkspace->text() );
  settings.setValue( baseKey + QStringLiteral( "/schema" ), txtSchema->text() );

  QDialog::accept();
}

void QgsOracleNewConnection::testConnection()
{
  QgsDataSourceUri uri;
  uri.setConnection( txtHost->text(), txtPort->text(), txtDatabase->text(), txtUsername->text(), txtPassword->text() );
  if ( !txtOptions->text().isEmpty() )
    uri.setParam( QStringLiteral( "dboptions" ), txtOptions->text() );
  if ( !txtWorkspace->text().isEmpty() )
    uri.setParam( QStringLiteral( "dbworkspace" ), txtWorkspace->text() );

  QgsOracleConn *conn = QgsOracleConnPool::instance()->acquireConnection( QgsOracleConn::toPoolName( uri ) );

  if ( conn )
  {
    // Database successfully opened; we can now issue SQL commands.
    bar->pushMessage( tr( "Connection to %1 was successful." ).arg( txtName->text() ),
                      Qgis::Info );
    // free connection resources
    QgsOracleConnPool::instance()->releaseConnection( conn );
  }
  else
  {
    bar->pushMessage( tr( "Connection failed - consult message log for details." ),
                      Qgis::Warning );
  }
}

void QgsOracleNewConnection::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#connecting-to-oracle-spatial" ) );
}