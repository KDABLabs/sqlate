-- Sqlite (which we use for this test) can't handle this...
-- ALTER TABLE tblExisting COLUMN column1 column1 varchar(128) NOT NULL;

-- so try something else instead that shows this code got executed:
ALTER TABLE tblExisting ADD COLUMN column3 varchar(32)
