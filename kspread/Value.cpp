/* This file is part of the KDE project
   Copyright (C) 2003,2004 Ariya Hidayat <ariya@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "Value.h"
#include "Doc.h"
#include "PointStorage.h"

#include <kdebug.h>

#include <QString>
#include <QTextStream>

#include <float.h>
#include <math.h>
#include <limits.h>

using namespace KSpread;

typedef PointStorage<Value*> ValueArray;


// helper class for Value
class KSpread::ValueData
{
  public:

    Value::Type type:4;
    Value::Format format:4;

    // reference count, at least one when object exists
    unsigned int count:24;

    union
    {
      bool b;
      long i;
      double f;
      QString* ps;
      ValueArray* pa;
    };

    // create empty data
    ValueData(): type( Value::Empty ),
      format (Value::fmt_None), count( 1 ), ps( 0 ) { };

    // destroys data
    ~ValueData(){ if( this == s_null ) s_null = 0;
       if( type == Value::Array ) delete pa;
       if( type == Value::String ) delete ps;
       if( type == Value::Error ) delete ps;
     }

    // static empty data to be shared
    static ValueData* null()
      { if( !s_null) s_null = new ValueData; else s_null->ref(); return s_null; }

    // increase reference count
    void ref() { count++; }

    // decrease reference count
    void unref()
      {  --count; if( !count ) delete this; }

    // true if it's null (which is shared)
    bool isNull(){ return this == s_null; }

    /** set most probable formatting based on the type */
    void setFormatByType ();

  private:

    static ValueData* s_null;
};

void KSpread::ValueData::setFormatByType ()
{
  switch (type) {
    case Value::Empty:
      format = Value::fmt_None;
    break;
    case Value::Boolean:
      format = Value::fmt_Boolean;
    break;
    case Value::Integer:
      format = Value::fmt_Number;
    break;
    case Value::Float:
      format = Value::fmt_Number;
    break;
    case Value::String:
      format = Value::fmt_String;
    break;
    case Value::Array:
      format = Value::fmt_None;
    break;
    case Value::CellRange:
      format = Value::fmt_None;
    break;
    case Value::Error:
      format = Value::fmt_String;
    break;
  };
}

// to be shared between all empty value
ValueData* ValueData::s_null = 0;

// static things
Value ks_value_empty;
Value ks_error_circle;
Value ks_error_depend;
Value ks_error_div0;
Value ks_error_na;
Value ks_error_name;
Value ks_error_null;
Value ks_error_num;
Value ks_error_parse;
Value ks_error_ref;
Value ks_error_value;

// create an empty value
Value::Value()
{
  d = ValueData::null();
}

// destructor
Value::~Value()
{
  d->unref();
}

// create value of certain type
Value::Value( Value::Type _type )
{
  d = new ValueData;
  d->type = _type;
  d->setFormatByType ();
}

// copy constructor
Value::Value( const Value& _value )
{
  d = ValueData::null();
  assign( _value );
}

// assignment operator
Value& Value::operator=( const Value& _value )
{
  return assign( _value );
}

// comparison operator - returns true only if strictly identical, unlike equal()/compare()
bool Value::operator==( const Value& v ) const
{
  const ValueData* n = v.d;
  if ( d->type != n->type )
    return false;
  switch( d->type )
  {
    case Empty: return true;
    case Boolean: return n->b == d->b;
    case Integer: return n->i == d->i;
    case Float:   return compare( n->f, d->f ) == 0;
    case String:  return *n->ps == *d->ps;
    case Array:   return *n->pa == *d->pa;
    case Error:   return *n->ps == *d->ps;
    default: break;
  }
  kWarning() << "Unhandled type in Value::operator==: " << d->type << endl;
  return false;
}

// create a boolean value
Value::Value( bool b )
{
  d = ValueData::null();
  setValue( b );
}

// create an integer value
Value::Value( long i )
{
  d = ValueData::null();
  setValue ( i );
}

// create an integer value
Value::Value( int i )
{
  d = ValueData::null();
  setValue ( i );
}

// create a floating-point value
Value::Value( double f )
{
  d = ValueData::null();
  setValue( f );
}

// create a string value
Value::Value( const QString& s )
{
  d = ValueData::null();
  setValue( s );
}

// create a string value
Value::Value (const char *s)
{
  d = ValueData::null();
  setValue (QString (s));
}

// create a floating-point value from date/time
Value::Value( const QDateTime& dt, const Doc* doc )
{
  d = ValueData::null();
  setValue( dt, doc );
}

// create a floating-point value from time
Value::Value( const QTime& dt, const Doc* doc )
{
  d = ValueData::null();
  setValue( dt, doc );
}

// create a floating-point value from date
Value::Value( const QDate& dt, const Doc* doc )
{
  d = ValueData::null();
  setValue( dt, doc );
}

// assign value from other
// shallow copy: only copy the data pointer
Value& Value::assign( const Value& _value )
{
  d->unref();
  d = _value.d;
  d->ref();
  return *this;
}

// return type of the value
Value::Type Value::type() const
{
  return d ? d->type : Empty;
}

// set the value to boolean
void Value::setValue( bool b )
{
  detach();
  d->type = Boolean;
  d->b = b;
  d->format = fmt_Boolean;
}

// get the value as boolean
bool Value::asBoolean() const
{
  bool result = false;

  if( type() == Value::Boolean )
    result = d->b;

  return result;
}

// set the value to integer
void Value::setValue( long i )
{
  detach();
  d->type = Integer;
  d->i = i;
  d->format = fmt_Number;
}

// set the value to integer
void Value::setValue( int i )
{
  detach();
  d->type = Integer;
  d->i = static_cast<long>( i );
  d->format = fmt_Number;
}

// get the value as integer
long Value::asInteger() const
{
  long result = 0;

  if( type() == Value::Integer )
    result = d->i;

  if( type() == Value::Float )
    result = static_cast<int>(floor( d->f ) );

  return result;
}

void Value::setValue( const Value& v )
{
  assign( v );
}

// set the value as floating-point
void Value::setValue( double f )
{
  detach();
  d->type = Float;
  d->f = f;
  d->format = fmt_Number;
}

// get the value as floating-point
double Value::asFloat() const
{
  double result = 0.0;

  if( type() == Value::Float )
    result = d->f;

  if( type() == Value::Integer )
    result = static_cast<double>(d->i);

  return result;
}

// set the value as string
void Value::setValue( const QString& s )
{
  detach();
  d->type = String;
  d->ps = new QString( s );
  d->format = fmt_String;
}

// get the value as string
QString Value::asString() const
{
  QString result;

  if( type() == Value::String )
  if( d->ps )
    result = QString( *d->ps );

  return result;
}

// set error message
void Value::setError( const QString& msg )
{
  detach();
  d->type = Error;
  d->ps = new QString( msg );
}

// get error message
QString Value::errorMessage() const
{
  QString result;

  if( type() == Value::Error )
  if( d->ps )
    result = QString( *d->ps );

  return result;
}

// set the value as date/time
// NOTE: date/time is stored as serial number
void Value::setValue( const QDateTime& dt, const Doc* doc  )
{
  const QDate refDate( doc->referenceDate() );
  const QTime refTime( 0, 0 );  // reference time is midnight

  double f= refDate.daysTo( dt.date() );
  f += static_cast<double>( refTime.msecsTo( dt.time() ) ) / 86400000.0; // 24*60*60*1000

  setValue( f );
  d->format = fmt_DateTime;
}

void Value::setValue( const QTime& time, const Doc* doc )
{
  Q_UNUSED( doc );
  const QTime refTime( 0, 0 );  // reference time is midnight

  const double f = static_cast<double>( refTime.msecsTo( time ) ) / 86400000.0; // 24*60*60*1000

  setValue( f );
  d->format = fmt_Time;
}

void Value::setValue( const QDate& date, const Doc* doc )
{
  const QDate refDate( doc->referenceDate() );

  const int i = refDate.daysTo( date );

  setValue( i );
  d->format = fmt_Date;
}

// get the value as date/time
QDateTime Value::asDateTime( const Doc* doc ) const
{
  QDateTime datetime( doc->referenceDate(), QTime(), Qt::UTC );

  const int days = asInteger();
  const int msecs = qRound( ( asFloat() - days ) * 86400000.0 ); // 24*60*60*1000
  datetime = datetime.addDays( days );
  datetime = datetime.addMSecs( msecs );

  return datetime;
}

// get the value as date
QDate Value::asDate( const Doc* doc ) const
{
  QDate dt( doc->referenceDate() );

  int i = asInteger();
  dt = dt.addDays( i );

  return dt;
}

// get the value as time
QTime Value::asTime( const Doc* doc ) const
{
  Q_UNUSED(doc);
  QTime dt;

  const int days = asInteger();
  const int msecs = qRound( ( asFloat() - days ) * 86400000.0 ); // 24*60*60*1000
  dt = dt.addMSecs( msecs );

  return dt;
}

Value::Format Value::format() const
{
  return d ? d->format : fmt_None;
}

void Value::setFormat (Value::Format fmt)
{
  detach();
  d->format = fmt;
}

Value Value::element( unsigned column, unsigned row ) const
{
  if( d->type != Array ) return *this;
  if( !d->pa ) return empty();
  Value* v = d->pa->lookup (column + 1, row + 1);
  return v ? Value( *v ) : empty();
}

Value Value::element( unsigned index ) const
{
  if( d->type != Array ) return *this;
  if( !d->pa ) return empty();
  Value* v = d->pa->data( index );
  return v ? Value( *v ) : empty();
}

void Value::setElement( unsigned column, unsigned row, const Value& v )
{
  if( d->type != Array ) return;
  if( !d->pa ) d->pa = new ValueArray();
  detach();
  d->pa->insert( column + 1, row + 1, new Value( v ) );
}

unsigned Value::columns() const
{
  if( d->type != Array ) return 1;
  if( !d->pa ) return 1;
  return d->pa->columns();
}

unsigned Value::rows() const
{
  if( d->type != Array ) return 1;
  if( !d->pa ) return 1;
  return d->pa->rows();
}

unsigned Value::count() const
{
  if( d->type != Array ) return 1;
  if( !d->pa ) return 1;
  return d->pa->count();
}

// reference to empty value
const Value& Value::empty()
{
  return ks_value_empty;
}

// reference to #CIRCLE! error
const Value& Value::errorCIRCLE()
{
  if( !ks_error_circle.isError() )
    ks_error_circle.setError( "#CIRCLE!" );
  return ks_error_circle;
}

// reference to #DEPEND! error
const Value& Value::errorDEPEND()
{
  if( !ks_error_depend.isError() )
    ks_error_depend.setError( "#DEPEND!" );
  return ks_error_depend;
}

// reference to #DIV/0! error
const Value& Value::errorDIV0()
{
  if( !ks_error_div0.isError() )
    ks_error_div0.setError( "#DIV/0!" );
  return ks_error_div0;
}

// reference to #N/A error
const Value& Value::errorNA()
{
  if( !ks_error_na.isError() )
    ks_error_na.setError( "#N/A" );
  return ks_error_na;
}

// reference to #NAME? error
const Value& Value::errorNAME()
{
  if( !ks_error_name.isError() )
    ks_error_name.setError( "#NAME?" );
  return ks_error_name;
}

// reference to #NUM! error
const Value& Value::errorNUM()
{
  if( !ks_error_num.isError() )
    ks_error_num.setError( "#NUM!" );
  return ks_error_num;
}

// reference to #NULL! error
const Value& Value::errorNULL()
{
  if( !ks_error_null.isError() )
    ks_error_null.setError( "#NULL!" );
  return ks_error_null;
}

// reference to #PARSE! error
const Value& Value::errorPARSE()
{
  if( !ks_error_parse.isError() )
    ks_error_parse.setError( "#PARSE!" );
  return ks_error_parse;
}

// reference to #REF! error
const Value& Value::errorREF()
{
  if( !ks_error_ref.isError() )
    ks_error_ref.setError( "#REF!" );
  return ks_error_ref;
}

// reference to #VALUE! error
const Value& Value::errorVALUE()
{
  if( !ks_error_value.isError() )
    ks_error_value.setError( "#VALUE!" );
  return ks_error_value;
}

// detach, create deep copy of ValueData
void Value::detach()
{
  if( d->isNull() || ( d->count > 1 ) )
  {
    ValueData* n;
    n = new ValueData;

    n->type = d->type;
    switch( n->type )
    {
    case Empty: break;
    case Boolean: n->b = d->b; break;
    case Integer: n->i = d->i; break;
    case Float:   n->f = d->f; break;
    case String:  n->ps = new QString( *d->ps ); break;
    case Array:   n->pa = new ValueArray; *n->pa = (*d->pa); break;
    case Error:   n->ps = new QString( *d->ps ); break;
    default: break;
    }

    d->unref();
    d = n;
  }
}

int Value::compare( double v1, double v2 )
{
  double v3 = v1 - v2;
  if( v3 > DBL_EPSILON ) return 1;
  if( v3 < -DBL_EPSILON ) return -1;
  return 0;
}

bool Value::isZero( double v )
{
  return fabs( v ) < DBL_EPSILON;
}

bool Value::isZero() const
{
  if( !isNumber() ) return false;
  return isZero( asFloat() );
}

bool Value::allowComparison( const Value& v ) const
{
  Value::Type t1 = d->type;
  Value::Type t2 = v.type();

  if( ( t1 == Empty ) && ( t2 == Empty ) ) return true;
  if( ( t1 == Empty ) && ( t2 == String ) ) return true;

  if( ( t1 == Boolean ) && ( t2 == Boolean ) ) return true;
  if( ( t1 == Boolean ) && ( t2 == Integer ) ) return true;
  if( ( t1 == Boolean ) && ( t2 == Float ) ) return true;
  if( ( t1 == Boolean ) && ( t2 == String ) ) return true;

  if( ( t1 == Integer ) && ( t2 == Boolean ) ) return true;
  if( ( t1 == Integer ) && ( t2 == Integer ) ) return true;
  if( ( t1 == Integer ) && ( t2 == Float ) ) return true;
  if( ( t1 == Integer ) && ( t2 == String ) ) return true;

  if( ( t1 == Float ) && ( t2 == Boolean ) ) return true;
  if( ( t1 == Float ) && ( t2 == Integer ) ) return true;
  if( ( t1 == Float ) && ( t2 == Float ) ) return true;
  if( ( t1 == Float ) && ( t2 == String ) ) return true;

  if( ( t1 == String ) && ( t2 == Empty ) ) return true;
  if( ( t1 == String ) && ( t2 == Boolean ) ) return true;
  if( ( t1 == String ) && ( t2 == Integer ) ) return true;
  if( ( t1 == String ) && ( t2 == Float ) ) return true;
  if( ( t1 == String ) && ( t2 == String ) ) return true;

  // errors can be compared too ...
  if ((t1 == Error) && (t2 == Error)) return true;

  return false;
}

// compare values. looks strange in order to be compatible with Excel
int Value::compare( const Value& v ) const
{
  Value::Type t1 = d->type;
  Value::Type t2 = v.type();

  // errors always less than everything else
  if( ( t1 == Error ) && ( t2 != Error ) )
    return -1;
  if( ( t2 == Error ) && ( t1 != Error ) )
    return 1;

  // comparing errors only yields 0 if they are the same
  if( ( t1 == Error ) && ( t2 == Error ) )
    return errorMessage() != v.errorMessage();

  // empty == empty
  if( ( t1 == Empty ) && ( t2 == Empty ) )
    return 0;

  // empty value is always less than string
  // (except when the string is empty)
  if( ( t1 == Empty ) && ( t2 == String ) )
    return( v.asString().isEmpty() ) ? 0 : -1;

  // boolean vs boolean
  if( ( t1 == Boolean ) && ( t2 == Boolean ) )
  {
    bool p = asBoolean();
    bool q = v.asBoolean();
    if( p ) return q ? 0 : 1;
    else return q ? -1 : 0;
  }

  // boolean is always greater than integer
  if( ( t1 == Boolean ) && ( t2 == Integer ) )
    return 1;

  // boolean is always greater than float
  if( ( t1 == Boolean ) && ( t2 == Float ) )
    return 1;

  // boolean is always greater than string
  if( ( t1 == Boolean ) && ( t2 == String ) )
    return 1;

  // integer is always less than boolean
  if( ( t1 == Integer ) && ( t2 == Boolean ) )
    return -1;

  // integer vs integer
  if( ( t1 == Integer ) && ( t2 == Integer ) )
  {
    long p = asInteger();
    long q = v.asInteger();
    return ( p == q ) ? 0 : ( p < q ) ? -1 : 1;
  }

  // integer vs float
  if( ( t1 == Integer ) && ( t2 == Float ) )
    return compare( asFloat(), v.asFloat() );

  // integer is always less than string
  if( ( t1 == Integer ) && ( t2 == String ) )
    return -1;

  // float is always less than boolean
  if( ( t1 == Float ) && ( t2 == Boolean ) )
    return -1;

  // float vs integer
  if( ( t1 == Float ) && ( t2 == Integer ) )
    return compare( asFloat(), v.asFloat() );

  // float vs float
  if( ( t1 == Float ) && ( t2 == Float ) )
    return compare( asFloat(), v.asFloat() );

  // float is always less than string
  if( ( t1 == Float ) && ( t2 == String ) )
    return -1;

  // string is always greater than empty value
  // (except when the string is empty)
  if( ( t1 == String ) && ( t2 == Empty ) )
    return( asString().isEmpty() ) ? 0 : 1;

  // string is always less than boolean
  if( ( t1 == String ) && ( t2 == Boolean ) )
    return -1;

  // string is always greater than integer
  if( ( t1 == String ) && ( t2 == Integer ) )
    return 1;

  // string is always greater than float
  if( ( t1 == String ) && ( t2 == Float ) )
    return 1;

  // The-Real-String comparison
  if( ( t1 == String ) && ( t2 == String ) )
    return asString().compare( v.asString() );

  // Undefined, actually allowComparison would return false
  return 0;
}

bool Value::equal( const Value& v ) const
{
  if (!allowComparison (v)) return false;
  return compare( v ) == 0;
}

bool Value::less( const Value& v ) const
{
  if (!allowComparison (v)) return false;
  return compare( v ) < 0;
}

bool Value::greater( const Value& v ) const
{
  if (!allowComparison (v)) return false;
  return compare( v ) > 0;
}

QTextStream& operator<<( QTextStream& ts, Value::Type type )
{
  switch( type )
  {
    case Value::Empty:   ts << "Empty"; break;
    case Value::Boolean: ts << "Boolean"; break;
    case Value::Integer: ts << "Integer"; break;
    case Value::Float:   ts << "Float"; break;
    case Value::String:  ts << "String"; break;
    case Value::Array:   ts << "Array"; break;
    case Value::Error:   ts << "Error"; break;
    default: ts << "Unknown!"; break;
  };
  return ts;
}

QTextStream& operator<<( QTextStream& ts, Value value )
{
  ts << value.type();
  switch( value.type() )
  {
    case Value::Empty:   break;

    case Value::Boolean:
      ts << ": ";
      if (value.asBoolean()) ts << "TRUE";
      else ts << "FALSE"; break;

    case Value::Integer:
      ts << ": " << value.asInteger(); break;

    case Value::Float:
      ts << ": " << value.asFloat(); break;

    case Value::String:
      ts << ": " << value.asString(); break;

    case Value::Array:
    {
      ts << ": {" << value.asString();
      const int cols = value.columns();
      const int rows = value.rows();
      for ( int row = 0; row < rows; ++row )
      {
        for ( int col = 0; col < cols; ++col )
        {
          ts << value.element(col,row);
          if ( col < cols-1 )
            ts << ';';
        }
        if ( row < rows-1 )
          ts << '|';
      }
      ts << '}';
      break;
    }

    case Value::Error:
      ts << '(' << value.errorMessage() << ')'; break;

    default: break;
  }
  return ts;
}
