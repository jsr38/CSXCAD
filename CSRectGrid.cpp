#include "CSRectGrid.h"
#include "tinyxml.h"
#include "fparser.hh"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

CSRectGrid::CSRectGrid(void)
{
	dDeltaUnit=1;
}

CSRectGrid::~CSRectGrid(void)
{
}

void CSRectGrid::AddDiscLine(int direct, double val)
{
	if ((direct>=0)&&(direct<3)) Lines[direct].push_back(val);
}

void CSRectGrid::AddDiscLines(int direct, int numLines, double* vals)
{
	for (int n=0;n<numLines;++n)
	{
		AddDiscLine(direct,vals[n]);
	}
}

string CSRectGrid::AddDiscLines(int direct, int numLines, double* vals, string DistFunction)
{
	if ((direct<0)||(direct>=3)) return string("Unknown grid direction!");
	if (DistFunction.empty()==false)
	{
		FunctionParser fParse;
		fParse.AddConstant("pi", 3.1415926535897932);
		string dirVar;
		switch (direct)
		{
		case 0:
			dirVar = "x";
			break;
		case 1:
			dirVar = "y";
			break;
		case 2:
			dirVar = "z";
			break;
		}
		fParse.Parse(DistFunction,dirVar);
		if (fParse.GetParseErrorType()!=FunctionParser::FP_NO_ERROR)
			return string("An error occured parsing f(") + dirVar + string(") - Parser message:\n") + string(fParse.ErrorMsg());

		double dValue=0;
		bool error=false;
		for (int n=0;n<numLines;++n)
		{
			dValue=fParse.Eval(&vals[n]);
			if (fParse.EvalError()!=0) error=true;
			AddDiscLine(direct,dValue);
		}
		if (error) return string("An error occured evaluation the grid function f(") + dirVar + string(")!");
	}
	return "";
}

bool CSRectGrid::RemoveDiscLine(int direct, int index)
{
	if ((direct<0) || (direct>=3)) return false;
	if ((index>=(int)Lines[direct].size()) || (index<0)) return false;
	vector<double>::iterator vIter=Lines[direct].begin();
	Lines[direct].erase(vIter+index);
	return true;
}

bool CSRectGrid::RemoveDiscLine(int direct, double val)
{
	if ((direct<0) || (direct>=3)) return false;
	for (size_t i=0;i<Lines[direct].size();++i)
	{
		if (Lines[direct].at(i)==val) return RemoveDiscLine(direct,(int)i);
	}
	return false;
}

void CSRectGrid::clear()
{
	Lines[0].clear();
	Lines[1].clear();
	Lines[2].clear();
	dDeltaUnit=1;
}

void CSRectGrid::ClearLines(int direct)
{
	if ((direct<0) || (direct>=3)) return;
	Lines[direct].clear();
}

double CSRectGrid::GetLine(int direct, size_t Index)
{
	if ((direct<0) || (direct>=3)) return 0;
	if (Lines[direct].size()<=Index) return 0;
	return Lines[direct].at(Index);
}

double* CSRectGrid::GetLines(int direct, double *array, unsigned int &qty, bool sorted)
{
	if ((direct<0) || (direct>=3)) return 0;
	if (sorted) Sort(direct);
	delete[] array;
	array = new double[Lines[direct].size()];
	for (size_t i=0;i<Lines[direct].size();++i) array[i]=Lines[direct].at(i);
	qty=Lines[direct].size();
	return array;
}

string CSRectGrid::GetLinesAsString(int direct)
{
	stringstream xStr;
	if ((direct<0)||(direct>=3)) return xStr.str();
	if (Lines[direct].size()>0)
	{
		for (size_t i=0;i<Lines[direct].size();++i)
		{
			if (i>0) xStr << ", ";
			xStr<<Lines[direct].at(i);
		}
	}
	return xStr.str();
}

int CSRectGrid::GetDimension()
{
	if (Lines[0].size()==0) return -1;
	if (Lines[1].size()==0) return -1;
	if (Lines[2].size()==0) return -1;
	int dim=0;
	if (Lines[0].size()>1) ++dim;
	if (Lines[1].size()>1) ++dim;
	if (Lines[2].size()>1) ++dim;
	return dim;
}

void CSRectGrid::IncreaseResolution(int nu, int factor)
{
	if ((nu<0) || (nu>=GetDimension())) return;
	if ((factor<=1) && (factor>9)) return;
	size_t size=Lines[nu].size();
	for (size_t i=0;i<size-1;++i)
	{
		double delta=(Lines[nu].at(i+1)-Lines[nu].at(i))/factor;
		for (int n=1;n<factor;++n)
		{
			AddDiscLine(nu,Lines[nu].at(i)+n*delta);
		}
	}
	Sort(nu);
}


void CSRectGrid::Sort(int direct)
{
	if ((direct<0) || (direct>=3)) return;
	vector<double>::iterator start = Lines[direct].begin();
	vector<double>::iterator end = Lines[direct].end();
	sort(start,end);
	end=unique(start,end);
	Lines[direct].erase(end,Lines[direct].end());
}

double* CSRectGrid::GetSimArea()
{
	for (int i=0;i<3;++i)
	{
		if (Lines[i].size()!=0)
		{
			SimBox[2*i]=*min_element(Lines[i].begin(),Lines[i].end());
			SimBox[2*i+1]=*max_element(Lines[i].begin(),Lines[i].end());
		}
		else SimBox[2*i]=SimBox[2*i+1]=0;
	}
	return SimBox;
}

bool CSRectGrid::Write2XML(TiXmlNode &root, bool sorted)
{
	if (sorted) {Sort(0);Sort(1);Sort(2);}
	TiXmlElement grid("RectilinearGrid");

	grid.SetDoubleAttribute("DeltaUnit",dDeltaUnit);

	TiXmlElement XLines("XLines");
	XLines.SetAttribute("Qty",(int)Lines[0].size());
	if (Lines[0].size()>0)
	{
		stringstream xStr;
		for (size_t i=0;i<Lines[0].size();++i)
		{
			if (i>0) xStr << ",";
			xStr<<Lines[0].at(i);
		}
		TiXmlText XText(xStr.str().c_str());
		XLines.InsertEndChild(XText);
	}
	grid.InsertEndChild(XLines);

	TiXmlElement YLines("YLines");
	YLines.SetAttribute("Qty",(int)Lines[1].size());
	if (Lines[1].size()>0)
	{
		stringstream yStr;
		for (size_t i=0;i<Lines[1].size();++i)
		{
			if (i>0) yStr << ",";
			yStr<<Lines[1].at(i);
		}
		TiXmlText YText(yStr.str().c_str());
		YLines.InsertEndChild(YText);
	}
	grid.InsertEndChild(YLines);

	TiXmlElement ZLines("ZLines");
	ZLines.SetAttribute("Qty",(int)Lines[2].size());
	if (Lines[2].size()>0)
	{
		stringstream zStr;
		for (size_t i=0;i<Lines[2].size();++i)
		{
			if (i>0) zStr << ",";
			zStr<<Lines[2].at(i);
		}
		TiXmlText ZText(zStr.str().c_str());
		ZLines.InsertEndChild(ZText);
	}
	grid.InsertEndChild(ZLines);

	root.InsertEndChild(grid);

	return true;
}

bool CSRectGrid::ReadFromXML(TiXmlNode &root)
{
	TiXmlElement* Lines=root.ToElement();
	if (Lines->QueryDoubleAttribute("DeltaUnit",&dDeltaUnit)!=TIXML_SUCCESS) dDeltaUnit=1.0;
	TiXmlNode* FN=NULL;
	TiXmlText* Text=NULL;
	string LineStr[3];
	size_t pos=0;

	Lines = root.FirstChildElement("XLines");
	if (Lines==NULL) return false;
	FN = Lines->FirstChild();
	if (FN!=NULL)
	{
		Text = FN->ToText();
		if (Text!=NULL)	LineStr[0]=string(Text->Value());
	}

	Lines = root.FirstChildElement("YLines");
	if (Lines==NULL) return false;
	FN = Lines->FirstChild();
	if (FN!=NULL)
	{
		Text = FN->ToText();
		if (Text!=NULL)	LineStr[1]=string(Text->Value());
	}

	Lines = root.FirstChildElement("ZLines");
	if (Lines==NULL) return false;
	FN = Lines->FirstChild();
	if (FN!=NULL)
	{
		Text = FN->ToText();
		if (Text!=NULL)	LineStr[2]=string(Text->Value());
	}
	string sub;

	for (int i=0;i<3;++i)
	{
		pos=0;
		do
		{
			pos=LineStr[i].find_first_of(',');
			if (pos==string::npos) pos=LineStr[i].size();
			else ++pos;
			sub=LineStr[i].substr(0,pos);
			//fprintf(stderr,"%d -> %s\n",pos,sub.c_str());
			if (sub.empty()==false) AddDiscLine(i,atof(sub.c_str()));
			LineStr[i].erase(0,pos);
		} while (LineStr[i].size()>0);
		Sort(i);
	}


	return true;
}
