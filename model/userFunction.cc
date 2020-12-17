/*
  @copyright Steve Keen 2020
  @author Russell Standish
  This file is part of Minsky.

  Minsky is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Minsky is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Minsky.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "expressionWalker.h"
#include "userFunction.h"
#include "evalOp.h"
#include "selection.h"
#include "minsky.h"
#include "minsky_epilogue.h"

namespace minsky
{
  int UserFunction::nextId=0;

  namespace {
    exprtk::parser<double> parser;
    exprtk::parser<UnitsExpressionWalker> unitsParser;
  }
  
  template <> void Operation<OperationType::userFunction>::iconDraw(cairo_t*) const
  {assert(false);}

  UserFunction::UserFunction() {
    description("uf"+std::to_string(nextId++)+"(x,y)");
    localSymbols.add_variable("x",x);
    localSymbols.add_variable("y",y);
    compiledExpression.register_symbol_table(externalSymbols);
    compiledExpression.register_symbol_table(localSymbols);
  }

  vector<string> UserFunction::externalVariables() const
  {
    // do an initial parse to pick up references to external variables
    exprtk::symbol_table<double> externalSymbols, localSymbols=this->localSymbols;
    exprtk::expression<double> compiledExpression;
    compiledExpression.register_symbol_table(externalSymbols);
    compiledExpression.register_symbol_table(localSymbols);
    parser.enable_unknown_symbol_resolver();
    parser.compile(expression, compiledExpression);
    parser.disable_unknown_symbol_resolver();
    std::vector<std::string> externalVariables;
    externalSymbols.get_variable_list(externalVariables);
    return externalVariables;
  }
  
  void UserFunction::compile()
  {
    // add them back in with their correct definitions
    externalSymbols.clear();
    for (auto& i: externalVariables())
      {
        auto v=minsky().variableValues.find(VariableValue::valueIdFromScope(group.lock(),i));
        if (v!=minsky().variableValues.end())
          externalSymbols.add_variable(i, (*v->second)[0]);
      }
    
      // TODO bind any other external references to the variableValues table
    if (!parser.compile(expression, compiledExpression))
      {
        string errorInfo;
        for (size_t i=0; i<parser.error_count(); ++i)
          errorInfo+=parser.get_error(i).diagnostic+'\n';
        throw_error("Invalid function expression:\n"+errorInfo);
      }
  }
  
  double UserFunction::evaluate(double in1, double in2)
  {
    x=in1, y=in2;
    return compiledExpression.value();
  }

  Units UserFunction::units(bool check) const
  {
    UnitsExpressionWalker x,y;
    x.units=ports[1]->units(check); x.check=check;
    y.units=ports[2]->units(check); y.check=check;

    vector<UnitsExpressionWalker> externalUnits;
    exprtk::symbol_table<UnitsExpressionWalker> symbolTable, unknownVariables;
    exprtk::expression<UnitsExpressionWalker> compiled;
    compiled.register_symbol_table(unknownVariables);
    compiled.register_symbol_table(symbolTable);
    symbolTable.add_variable("x",x);
    symbolTable.add_variable("y",y);

    std::vector<std::string> externalIds=const_cast<UserFunction*>(this)->externalVariables();

    externalUnits.reserve(externalIds.size());
    for (auto& i: externalIds)
      {
        auto v=minsky().variableValues.find(VariableValue::valueIdFromScope(group.lock(),i));
        if (v!=minsky().variableValues.end())
          {
            externalUnits.emplace_back();
            externalUnits.back().units=v->second->units;
            externalUnits.back().check=check;
            unknownVariables.add_variable(i, externalUnits.back());
          }
        else
          if (check)
            throw_error("unknown variable: "+i);
          else
            return {};
      }

    try
      {
        unitsParser.compile(expression, compiled);
        return compiled.value().units;
      }
    catch (const std::exception& ex)
      {
        if (check)
          throw_error(ex.what());
        else
          return {};
      }
  }

  
}

