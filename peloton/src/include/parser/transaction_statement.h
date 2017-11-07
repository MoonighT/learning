//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// statement_transaction.h
//
// Identification: src/include/parser/statement_transaction.h
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include "parser/sql_statement.h"
#include "common/sql_node_visitor.h"

namespace peloton {
namespace parser {

/**
 * @class TransactionStatement
 * @brief Represents "BEGIN or COMMIT or ROLLBACK [TRANSACTION]"
 */
class TransactionStatement : public SQLStatement {
 public:
  enum CommandType {
    kBegin,
    kCommit,
    kRollback,
  };

  TransactionStatement(CommandType type)
      : SQLStatement(StatementType::TRANSACTION), type(type) {}

  virtual void Accept(SqlNodeVisitor* v) const override {
    v->Visit(this);
  }

  CommandType type;
};

}  // namespace parser
}  // namespace peloton
