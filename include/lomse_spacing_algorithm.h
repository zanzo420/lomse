//---------------------------------------------------------------------------------------
// This file is part of the Lomse library.
// Lomse is copyrighted work (c) 2010-2016. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice, this
//      list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above copyright notice, this
//      list of conditions and the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// For any comment, suggestion or feature request, please contact the manager of
// the project at cecilios@users.sourceforge.net
//---------------------------------------------------------------------------------------

#ifndef __LOMSE_SPACING_ALGORITHM_H__        //to avoid nested includes
#define __LOMSE_SPACING_ALGORITHM_H__

#include "lomse_basic.h"
#include "lomse_logger.h"

//std
#include <list>
using namespace std;

namespace lomse
{

//forward declarations
class ImoScore;
class ScoreMeter;
class LibraryScope;
class ScoreLayouter;
class ShapesStorage;
class ShapesCreator;
class PartsEngraver;
class GmoBoxSlice;
class ColStaffObjsEntry;
class TimeGridTable;
class GmoBoxSliceInstr;

class ColumnsBuilder;
class StaffObjsCursor;
class ColumnBreaker;
class ImoInstrument;
class ImoStaffObj;
class GmoShape;
class ColumnData;

//---------------------------------------------------------------------------------------
// SpacingAlgorithm
// Abstract class providing the public interface for any spacing algorithm.
// The idea is to facilitate testing different algorithms without having to
// rewrite other parts of the code.
//
class SpacingAlgorithm
{
protected:
    LibraryScope&   m_libraryScope;
    ScoreMeter*     m_pScoreMeter;
    ScoreLayouter*  m_pScoreLyt;
    ImoScore*       m_pScore;
    ShapesStorage&  m_shapesStorage;
    ShapesCreator*  m_pShapesCreator;
    PartsEngraver*  m_pPartsEngraver;

public:
    SpacingAlgorithm(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                     ScoreLayouter* pScoreLyt, ImoScore* pScore,
                     ShapesStorage& shapesStorage, ShapesCreator* pShapesCreator,
                     PartsEngraver* pPartsEngraver);
    virtual ~SpacingAlgorithm();

    //spacing algorithm

    ///This is the first method to be invoked. Your implementation must:
    ///
    ///- collect score content and organize it as necessary for the algorithm.
    ///
    ///- split the content into columns (e.g. measures). A column must end in a point
    ///  were it must be possible to break the lines. Splitting the content in measures
    ///  is the most simple approach, but smaller chunks could be possible (and
    ///  to deal with scores without barlines, with multimetrics scores and with
    ///  long measures.
    ///
    virtual void split_content_in_columns() = 0;

    ///Next, this method will be invoked. Your implementation must:
    ///
    ///- apply the spacing algorithm for determining the minimum size of each column.
    ///
    ///- assign a penalty factor to each column, for the line break algorithm.
    ///
    virtual void do_spacing_algorithm() = 0;

    //provide information

    ///Return the number of columns in which the content has been split
    virtual int get_num_columns() = 0;

    virtual LUnits get_staves_height() = 0;

    //invoked from system layouter
    virtual LUnits aditional_space_before_adding_column(int iCol) = 0;
    virtual LUnits get_column_width(int iCol, bool fFirstColumnInSystem) = 0;
    virtual void reposition_slices_and_staffobjs(int iFirstCol, int iLastCol,
            LUnits yShift,
            LUnits* yMin, LUnits* yMax) = 0;
    virtual void justify_system(int iFirstCol, int iLastCol, LUnits uSpaceIncrement) = 0;

    //for line break algorithm
    virtual bool is_empty_column(int iCol) = 0;
    virtual float get_penalty_factor(int iCol) = 0;

    //information about a column
    virtual LUnits get_trimmed_width(int iCol) = 0;
    virtual bool column_has_barline(int iCol) = 0;
    virtual bool has_system_break(int iCol) = 0;

    //boxes and shapes
    virtual void add_shapes_to_boxes(int iCol, ShapesStorage* pStorage) = 0;
    virtual void delete_shapes(int iCol) = 0;
    virtual GmoBoxSliceInstr* get_slice_instr(int iCol, int iInstr) = 0;
    virtual void set_slice_final_position(int iCol, LUnits left, LUnits top) = 0;
    virtual void delete_box_and_shapes(int iCol) = 0;
    ///store slice box for column iCol and access it
    virtual void set_slice_box(int iCol, GmoBoxSlice* pBoxSlice) = 0;
    virtual GmoBoxSlice* get_slice_box(int iCol) = 0;

    //methods to compute results
    virtual TimeGridTable* create_time_grid_table_for_column(int iCol) = 0;

    //access to info
    virtual ColStaffObjsEntry* get_prolog_clef(int iCol, ShapeId idx) = 0;
    virtual ColStaffObjsEntry* get_prolog_key(int iCol, ShapeId idx) = 0;

    //debug
    virtual void dump_column_data(int iCol, ostream& outStream) = 0;
    virtual void set_trace_level(int iCol, int nTraceLevel) = 0;
};


//---------------------------------------------------------------------------------------
// SpAlgColumn
// Abstract class for spacing algorithms based on using ColumnBuilder object for
// organizing the content in columns and managing the columns information.
//
class SpAlgColumn: public SpacingAlgorithm
{
protected:
    LibraryScope&   m_libraryScope;
    ScoreMeter*     m_pScoreMeter;
    ScoreLayouter*  m_pScoreLyt;
    ImoScore*       m_pScore;
    ShapesStorage&  m_shapesStorage;
    ShapesCreator*  m_pShapesCreator;
    PartsEngraver*  m_pPartsEngraver;
    ColumnsBuilder* m_pColsBuilder;
    std::vector<ColumnData*> m_colsData;


public:
    SpAlgColumn(LibraryScope& libraryScope, ScoreMeter* pScoreMeter,
                ScoreLayouter* pScoreLyt, ImoScore* pScore,
                ShapesStorage& shapesStorage, ShapesCreator* pShapesCreator,
                PartsEngraver* pPartsEngraver);
    virtual ~SpAlgColumn();


    //overrides of base class SpacingAlgorithm. Normally, not need to override them
    //------------------------------------------------------------------------------

    //collect content
    void split_content_in_columns();
    //spacing algorithm
    void do_spacing_algorithm();
    //boxes and shapes
    virtual void add_shapes_to_boxes(int iCol, ShapesStorage* pStorage);
    virtual GmoBoxSliceInstr* get_slice_instr(int iCol, int iInstr);
    virtual void set_slice_final_position(int iCol, LUnits left, LUnits top);
    LUnits get_staves_height();
    ///store slice box for column iCol and access it
    virtual void set_slice_box(int iCol, GmoBoxSlice* pBoxSlice);
    virtual GmoBoxSlice* get_slice_box(int iCol);
    virtual bool has_system_break(int iCol);
    virtual void delete_box_and_shapes(int iCol);


    //methods in base class SpacingAlgorithm that still need to be created
    //------------------------------------------------------------------------

    //invoked from system layouter
    virtual LUnits aditional_space_before_adding_column(int iCol) = 0;
    virtual LUnits get_column_width(int iCol, bool fFirstColumnInSystem) = 0;
    virtual void reposition_slices_and_staffobjs(int iFirstCol, int iLastCol,
            LUnits yShift,
            LUnits* yMin, LUnits* yMax) = 0;
    virtual void justify_system(int iFirstCol, int iLastCol, LUnits uSpaceIncrement) = 0;

    //for line break algorithm
    virtual bool is_empty_column(int iCol) = 0;
    virtual float get_penalty_factor(int iCol) = 0;

    //information about a column
    virtual LUnits get_trimmed_width(int iCol) = 0;
    virtual bool column_has_barline(int iCol) = 0;

    //methods to compute results
    virtual TimeGridTable* create_time_grid_table_for_column(int iCol) = 0;

    //debug
    virtual void dump_column_data(int iCol, ostream& outStream) = 0;


    //new methods to be implemented by derived classes (apart from previous methods)
    //---------------------------------------------------------------------------------

    //column creation: collecting content
    ///start a new column and prepare for receiving information
    virtual void start_column_measurements(int iCol, LUnits uxStart, LUnits fixedSpace) = 0;
    ///save information for staff object in current column
    virtual void include_object(ColStaffObjsEntry* pCurEntry, int iCol, int iLine, int iInstr, ImoStaffObj* pSO,
                                TimeUnits rTime, int nStaff, GmoShape* pShape,
                                bool fInProlog=false) = 0;
    ///terminate current column
    virtual void finish_column_measurements(int iCol, LUnits xStart) = 0;

    //spacing algorithm main actions
    ///apply spacing algorithm to column iCol
    virtual void do_spacing(int iCol, bool fTrace=false, int level=k_trace_off) = 0;
    ///determine minimum required width for column iCol
    virtual void assign_width_to_column(int iCol) = 0;

    //get results: info about a column
    ///the column ends with a visible barline
    virtual bool column_has_visible_barline(int iCol) = 0;

    //auxiliary: shapes and boxes
    ///add shapes for staff objects to graphical model
    virtual void add_shapes_to_box(int iCol, GmoBoxSliceInstr* pSliceInstrBox,
                                   int iInstr) = 0;

    virtual void delete_shapes(int iCol) = 0;


    //normally, no need to override
    //-------------------------------------------------------------------------

    ///Returns the number of columns in which the content has been split
    virtual int get_num_columns();

    ///save context information (clef, key) for iCol, and access it
    virtual void save_context(int iCol, int iInstr, int iStaff,
                              ColStaffObjsEntry* pClefEntry,
                              ColStaffObjsEntry* pKeyEntry);
    virtual ColStaffObjsEntry* get_prolog_clef(int iCol, ShapeId idx);
    virtual ColStaffObjsEntry* get_prolog_key(int iCol, ShapeId idx);

    ///system break found while collecting content for iCol
    virtual void set_system_break(int iCol, bool value);

    ///a new column is going to be created (do whatever your spacing algorithm requires:
    ///allocating memory for column data, etc.)
    virtual void prepare_for_new_column(int UNUSED(iCol)) {}

    ///create slice instr box for column iCol and access it
    virtual GmoBoxSliceInstr* create_slice_instr(int iCol, ImoInstrument* pInstr,
            LUnits yTop);
    ///set width of slice box for column iCol
    virtual void set_slice_width(int iCol, LUnits width);

    ///activate trace for iCol at level traceLevel
    virtual void set_trace_level(int iCol, int nTraceLevel);


};


//---------------------------------------------------------------------------------------
// ColumnsBuilder: algorithm to build the columns for one score
class ColumnsBuilder
{
protected:
    ScoreMeter*     m_pScoreMeter;
    ScoreLayouter*  m_pScoreLyt;
    ImoScore*       m_pScore;
    ShapesStorage&  m_shapesStorage;
    ShapesCreator*  m_pShapesCreator;
    PartsEngraver*  m_pPartsEngraver;
    StaffObjsCursor* m_pSysCursor;  //cursor for traversing the score
    ColumnBreaker*  m_pBreaker;
    std::vector<LUnits> m_SliceInstrHeights;
    LUnits m_stavesHeight;      //system height without top and bottom margins
    UPoint m_pagePos;           //to track current position

    int m_iColumn;   //[0..n-1] current column. (-1 if no column yet created!)

    int m_iColumnToTrace;   //support for debug and unit test
    int m_nTraceLevel;

    SpAlgColumn* m_pSpAlgorithm;
    int m_numColumns;
    std::vector<ColumnData*>& m_colsData;

public:
    ColumnsBuilder(ScoreMeter* pScoreMeter, vector<ColumnData*>& colsData,
                   ScoreLayouter* pScoreLyt, ImoScore* pScore,
                   ShapesStorage& shapesStorage,
                   ShapesCreator* pShapesCreator,
                   PartsEngraver* pPartsEngraver,
                   SpAlgColumn* pSpAlgorithm);
    ~ColumnsBuilder();


    void create_columns();
    void do_spacing_algorithm();
    inline LUnits get_staves_height()
    {
        return m_stavesHeight;
    }

    //support for debuggin and unit tests
    inline void set_debug_options(int iCol, int level)
    {
        m_iColumnToTrace = iCol;
        m_nTraceLevel = level;
    }

    //managing shapes
    void add_shapes_to_boxes(int iCol, ShapesStorage* pStorage);
    void delete_shapes(int iCol);


protected:
    void determine_staves_vertical_position();

    void prepare_for_new_column();
    void create_column_boxes();
    void collect_content_for_this_column();
    void layout_column();
    void assign_width_to_column();

    GmoBoxSlice* create_slice_box();
    void find_and_save_context_info_for_this_column();

    LUnits determine_initial_fixed_space();

    void store_info_about_attached_objects(ImoStaffObj* pSO, GmoShape* pShape,
                                           int iInstr, int iStaff, int iCol, int iLine,
                                           ImoInstrument* pInstr);

    bool determine_if_is_in_prolog(TimeUnits rTime);
    inline bool is_first_column()
    {
        return m_iColumn == 0;
    }

};


}   //namespace lomse

#endif    // __LOMSE_SPACING_ALGORITHM_H__

