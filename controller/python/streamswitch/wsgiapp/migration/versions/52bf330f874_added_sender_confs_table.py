"""Added sender_confs table

Revision ID: 52bf330f874
Revises: 46df8acc101
Create Date: 2015-12-06 10:58:09.525516

"""

# revision identifiers, used by Alembic.
revision = '52bf330f874'
down_revision = '46df8acc101'
branch_labels = None
depends_on = None

from alembic import op
import sqlalchemy as sa


def upgrade():
    ### commands auto generated by Alembic - please adjust! ###
    op.create_table('sender_confs',
    sa.Column('sender_type', sa.String(convert_unicode=True), nullable=False),
    sa.Column('sender_name', sa.String(convert_unicode=True), nullable=False),
    sa.Column('dest_url', sa.String(convert_unicode=True), nullable=False),
    sa.Column('dest_format', sa.String(convert_unicode=True), server_default='', nullable=False),
    sa.Column('stream_name', sa.String(convert_unicode=True), server_default='', nullable=False),
    sa.Column('stream_host', sa.String(convert_unicode=True), server_default='', nullable=False),
    sa.Column('stream_port', sa.Integer(), server_default='0', nullable=False),
    sa.Column('log_file', sa.String(convert_unicode=True), nullable=True),
    sa.Column('log_size', sa.Integer(), server_default='1048576', nullable=False),
    sa.Column('log_rotate', sa.Integer(), server_default='3', nullable=False),
    sa.Column('log_level', sa.Integer(), server_default='6', nullable=False),
    sa.Column('err_restart_interval', sa.Float(), server_default='30.0', nullable=False),
    sa.Column('age_time', sa.Float(), server_default='0.0', nullable=False),
    sa.Column('extra_options_json', sa.Text(convert_unicode=True), nullable=False),
    sa.Column('other_kwargs_json', sa.Text(convert_unicode=True), nullable=False),
    sa.PrimaryKeyConstraint('sender_name')
    )
    ### end Alembic commands ###


def downgrade():
    ### commands auto generated by Alembic - please adjust! ###
    op.drop_table('sender_confs')
    ### end Alembic commands ###
