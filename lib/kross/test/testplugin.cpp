/***************************************************************************
 * testplugin.cpp
 * This file is part of the KDE project
 * copyright (C)2005 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "testplugin.h"
#include "testobject.h"

#include <kdebug.h>

/************************************************************************
 * TestPluginObject
 */

TestPluginObject::TestPluginObject(const QString& name)
    : Kross::Api::Class<TestPluginObject>(name)
{
    this->addFunction("internalfunc1",
        new Kross::Api::ProxyFunction <
            TestPluginObject,
            Kross::Api::ProxyValue< Kross::Api::Variant, uint >, // returnvalue
            Kross::Api::ProxyValue< Kross::Api::Variant, uint > // first argument
            > ( this, &TestPluginObject::internalfunc1 )
    );

    addFunction("func1", &TestPluginObject::func1);
    addFunction("func2", &TestPluginObject::func2,
        Kross::Api::ArgumentList()
            << Kross::Api::Argument("Kross::Api::Variant::String")
            << Kross::Api::Argument("Kross::Api::Variant::String") );
    addFunction("func3", &TestPluginObject::func3,
        Kross::Api::ArgumentList()
            << Kross::Api::Argument("Kross::Api::Variant::String")
            << Kross::Api::Argument("Kross::Api::Variant::String") );
    addFunction("func4", &TestPluginObject::func4,
        Kross::Api::ArgumentList()
            << Kross::Api::Argument("Kross::Api::Variant::String")
            << Kross::Api::Argument("Kross::Api::Variant::String")
            << Kross::Api::Argument("Kross::Api::Variant::String") );
    addFunction("func5", &TestPluginObject::func5,
        Kross::Api::ArgumentList()
            << Kross::Api::Argument("Kross::Api::Variant::String")
            << Kross::Api::Argument("Kross::Api::Variant::String")
            << Kross::Api::Argument("Kross::Api::Variant::String") );
    addFunction("func6", &TestPluginObject::func6,
        Kross::Api::ArgumentList()
            << Kross::Api::Argument("Kross::Api::List") );
    addFunction("func7", &TestPluginObject::func7,
        Kross::Api::ArgumentList()
            << Kross::Api::Argument("Kross::Api::List")
            << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("func8", &TestPluginObject::func8,
        Kross::Api::ArgumentList()
            << Kross::Api::Argument("Kross::Api::Variant")
            << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("func9", &TestPluginObject::func9,
        Kross::Api::ArgumentList()
            << Kross::Api::Argument("Kross::Api::Variant")
            << Kross::Api::Argument("Kross::Api::Variant") );

}

TestPluginObject::~TestPluginObject()
{
}

const QString TestPluginObject::getClassName() const
{
    return "TestPluginObject";
}

uint TestPluginObject::internalfunc1(uint i)
{
    kdDebug() << "CALLED => TestPluginObject::internalfunc1" << endl;
    return i;
}

Kross::Api::Object::Ptr TestPluginObject::func1(Kross::Api::List::Ptr /*args*/)
{
    //kdDebug() << "CALLED => TestPluginObject::func1 args=" << args->toString() << endl;
    return 0;
}

Kross::Api::Object::Ptr TestPluginObject::func2(Kross::Api::List::Ptr /*args*/)
{
    //kdDebug() << "CALLED => TestPluginObject::func2 args=" << args->toString() << endl;
    return new Kross::Api::Variant("func2returnvalue");
}

Kross::Api::Object::Ptr TestPluginObject::func3(Kross::Api::List::Ptr /*args*/)
{
    //kdDebug() << "CALLED => TestPluginObject::func3 args=" << args->toString() << endl;
    return new Kross::Api::Variant("func3returnvalue");
}

Kross::Api::Object::Ptr TestPluginObject::func4(Kross::Api::List::Ptr /*args*/)
{
    //kdDebug() << "CALLED => TestPluginObject::func4 args=" << args->toString() << endl;
    return new Kross::Api::Variant("func4returnvalue");
}

Kross::Api::Object::Ptr TestPluginObject::func5(Kross::Api::List::Ptr /*args*/)
{
    //kdDebug() << "CALLED => TestPluginObject::func5 args=" << args->toString() << endl;
    return new Kross::Api::Variant("func5returnvalue");
}

Kross::Api::Object::Ptr TestPluginObject::func6(Kross::Api::List::Ptr args)
{
    Kross::Api::List* list = Kross::Api::Object::fromObject< Kross::Api::List >( args->item(0) );
    return list->item(0);
}

Kross::Api::Object::Ptr TestPluginObject::func7(Kross::Api::List::Ptr args)
{
    return args->item(0);
}

Kross::Api::Object::Ptr TestPluginObject::func8(Kross::Api::List::Ptr args)
{
    return args->item(0);
}

Kross::Api::Object::Ptr TestPluginObject::func9(Kross::Api::List::Ptr args)
{
    return args->item(0);
}

/************************************************************************
 * TestPluginModule
 */

TestPluginModule::TestPluginModule(const QString& name)
    : Kross::Api::Module(name)
    , m_testobject( new TestObject() )

{
    addChild( new TestPluginObject("testpluginobject1") );

    // Let's wrap a whole instance and it's methodfunctions.
    Kross::Api::ProxyClass<TestObject> *proxyclass =
        new Kross::Api::ProxyClass<TestObject>("testpluginobject2", m_testobject);
    addChild(proxyclass);

    proxyclass->addFunction("func1",
        new Kross::Api::ProxyFunction <
            TestObject,
            Kross::Api::ProxyValue< Kross::Api::Variant, uint >, // returnvalue
            Kross::Api::ProxyValue< Kross::Api::Variant, uint > // first argument
            > ( m_testobject, &TestObject::func1 )
    );
    proxyclass->addFunction("func2",
        new Kross::Api::ProxyFunction <
            TestObject,
            Kross::Api::ProxyValue< Kross::Api::Variant, void >, // returnvalue
            Kross::Api::ProxyValue< Kross::Api::Variant, QString >, // first argument
            Kross::Api::ProxyValue< Kross::Api::Variant, int > // second argument
            > ( m_testobject, &TestObject::func2 )
    );
    proxyclass->addFunction("func3",
        new Kross::Api::ProxyFunction<
            TestObject,
            Kross::Api::ProxyValue< Kross::Api::Variant, QString >, // returnvalue
            Kross::Api::ProxyValue< Kross::Api::Variant, QString >, // first argument
            Kross::Api::ProxyValue< Kross::Api::Variant, int > // second argument
            > ( m_testobject, &TestObject::func3) );
    proxyclass->addFunction("func4",
        new Kross::Api::ProxyFunction<
            TestObject,
            Kross::Api::ProxyValue< Kross::Api::Variant, const QString& >, // returnvalue
            Kross::Api::ProxyValue< Kross::Api::Variant, const QString& >, // first argument
            Kross::Api::ProxyValue< Kross::Api::Variant, int > // second argument
            > ( m_testobject, &TestObject::func4) );

}

TestPluginModule::~TestPluginModule()
{
    delete m_testobject;
}

const QString TestPluginModule::getClassName() const
{
    return "TestPluginModule";
}

