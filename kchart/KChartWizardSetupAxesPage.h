#ifndef _KCHARTWIZARDSETUPAXESPAGE_H
#define _KCHARTWIZARDSETUPAXESPAGE_H

#include <qwidget.h>
#include <qstring.h>

class KChartDoc;
class KChartWidget;

class KChartWizardSetupAxesPage : public QWidget
{
  Q_OBJECT

public:
  KChartWizardSetupAxesPage( QWidget* parent, KChartDoc* chart );
  ~KChartWizardSetupAxesPage();

public slots:
  void setTickLength( const QString & newValue );
  void setYTicksNum( const QString & newValue );
  void setXLabelSkip( const QString & newValue );
  void setYLabelSkip( const QString & newValue );

private:
  KChartWidget* preview;
  KChartDoc* _chart;
};

#endif
