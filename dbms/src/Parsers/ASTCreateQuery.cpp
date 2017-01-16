#include <DB/Parsers/ASTCreateQuery.h>

namespace DB {

String ASTCreateQuery::getID() const
{
	return (attach ? "AttachQuery_" : "CreateQuery_") + database + "_" + table;
};

ASTPtr ASTCreateQuery::clone() const
{
	auto res = std::make_shared<ASTCreateQuery>(*this);
	res->children.clear();

	if (columns) 	{ res->columns = columns->clone(); 	res->children.push_back(res->columns); }
	if (storage) 	{ res->storage = storage->clone(); 	res->children.push_back(res->storage); }
	if (select) 	{ res->select = select->clone(); 	res->children.push_back(res->select); }
	if (inner_storage) 	{ res->inner_storage = inner_storage->clone(); 	res->children.push_back(res->inner_storage); }

	return res;
}

void ASTCreateQuery::formatImpl(const FormatSettings & settings, FormatState & state, FormatStateStacked frame) const
{
	frame.need_parens = false;

	if (!database.empty() && table.empty())
	{
		settings.ostr << (settings.hilite ? hilite_keyword : "")
			<< (attach ? "ATTACH DATABASE " : "CREATE DATABASE ")
			<< (if_not_exists ? "IF NOT EXISTS " : "")
			<< (settings.hilite ? hilite_none : "")
			<< backQuoteIfNeed(database);

		if (storage)
		{
			settings.ostr << (settings.hilite ? hilite_keyword : "") << " ENGINE" << (settings.hilite ? hilite_none : "") << " = ";
			storage->formatImpl(settings, state, frame);
		}

		return;
	}

	{
		std::string what = "TABLE";
		if (is_view)
			what = "VIEW";
		if (is_materialized_view)
			what = "MATERIALIZED VIEW";

		settings.ostr
			<< (settings.hilite ? hilite_keyword : "")
				<< (attach ? "ATTACH " : "CREATE ")
				<< (is_temporary ? "TEMPORARY " : "")
				<< what
				<< " " << (if_not_exists ? "IF NOT EXISTS " : "")
			<< (settings.hilite ? hilite_none : "")
			<< (!database.empty() ? backQuoteIfNeed(database) + "." : "") << backQuoteIfNeed(table);

		if (!cluster.empty())
		{
			settings.ostr
				<< (settings.hilite ? hilite_keyword : "")
				<< " ON CLUSTER "
				<< (settings.hilite ? hilite_none : "")
				<< backQuoteIfNeed(cluster);
		}
	}

	if (!as_table.empty())
	{
		settings.ostr << (settings.hilite ? hilite_keyword : "") << " AS " << (settings.hilite ? hilite_none : "")
		<< (!as_database.empty() ? backQuoteIfNeed(as_database) + "." : "") << backQuoteIfNeed(as_table);
	}

	if (columns)
	{
		settings.ostr << (settings.one_line ? " (" : "\n(");
		FormatStateStacked frame_nested = frame;
		++frame_nested.indent;
		columns->formatImpl(settings, state, frame_nested);
		settings.ostr << (settings.one_line ? ")" : "\n)");
	}

	if (storage && !is_materialized_view && !is_view)
	{
		settings.ostr << (settings.hilite ? hilite_keyword : "") << " ENGINE" << (settings.hilite ? hilite_none : "") << " = ";
		storage->formatImpl(settings, state, frame);
	}

	if (inner_storage)
	{
		settings.ostr << (settings.hilite ? hilite_keyword : "") << " ENGINE" << (settings.hilite ? hilite_none : "") << " = ";
		inner_storage->formatImpl(settings, state, frame);
	}

	if (is_populate)
	{
		settings.ostr << (settings.hilite ? hilite_keyword : "") << " POPULATE" << (settings.hilite ? hilite_none : "");
	}

	if (select)
	{
		settings.ostr << (settings.hilite ? hilite_keyword : "") << " AS" << settings.nl_or_ws << (settings.hilite ? hilite_none : "");
		select->formatImpl(settings, state, frame);
	}
}

}